/*
 * rfbserver.c - deal with server-side of the RFB protocol.
 */

/*
 *  Copyright (C) 2005 Rohit Kumar, Johannes E. Schindelin
 *  Copyright (C) 2002 RealVNC Ltd.
 *  OSXvnc Copyright (C) 2001 Dan McGuirk <mcguirk@incompleteness.net>.
 *  Original Xvnc code Copyright (C) 1999 AT&T Laboratories Cambridge.
 *  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

#define _CRT_SECURE_NO_WARNINGS
#ifdef __STRICT_ANSI__
#define _BSD_SOURCE
#endif
#include <string.h>
#include <rfb/rfb.h>
#include <rfb/rfbregion.h>
#include "private.h"
#include "defines.h"

#include <shlwapi.h>
#include <Shlobj.h>

#ifdef LIBVNCSERVER_HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef WIN32
#define x_write(sock,buf,len) send(sock,buf,len,0)
#define S_ISDIR(m)	(((m)&_S_IFMT) == _S_IFDIR)
#else
#ifdef LIBVNCSERVER_HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <pwd.h>
#ifdef LIBVNCSERVER_HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef LIBVNCSERVER_HAVE_NETINET_IN_H
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#endif
#endif

#ifdef DEBUGPROTO
#undef DEBUGPROTO
#define DEBUGPROTO(x) x
#else
#define DEBUGPROTO(x)
#endif
#include <stdarg.h>
#include <scale.h>
/* stst() */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
/* readdir() */
/* errno */
#include <errno.h>
/* strftime() */
#include <time.h>
#define CFF_RECURSE 0x1

static void rfbProcessClientProtocolVersion(rfbClientPtr cl);
static void rfbProcessClientNormalMessage(rfbClientPtr cl);
static void rfbProcessClientInitMessage(rfbClientPtr cl);

#ifdef LIBVNCSERVER_HAVE_LIBPTHREAD
void rfbIncrClientRef(rfbClientPtr cl)
{
    LOCK(cl->refCountMutex);
    cl->refCount++;
    UNLOCK(cl->refCountMutex);
}

void rfbDecrClientRef(rfbClientPtr cl)
{
    LOCK(cl->refCountMutex);
    cl->refCount--;
    if(cl->refCount<=0) /* just to be sure also < 0 */
        TSIGNAL(cl->deleteCond);
    UNLOCK(cl->refCountMutex);
}
#else
void rfbIncrClientRef(rfbClientPtr cl) {}
void rfbDecrClientRef(rfbClientPtr cl) {}
#endif

#ifdef LIBVNCSERVER_HAVE_LIBPTHREAD
static MUTEX(rfbClientListMutex);
#endif

struct rfbClientIterator
{
    rfbClientPtr next;
    rfbScreenInfoPtr screen;
    rfbBool closedToo;
};

void
rfbClientListInit(rfbScreenInfoPtr rfbScreen)
{
    /**if(sizeof(rfbBool)!=1) {
        /* a sanity check /
        fprintf(stderr,"rfbBool's size is not 1 (%d)!\n",(int)sizeof(rfbBool));
    /* we cannot continue, because rfbBool is supposed to be char everywhere /
    exit(1);
    }*/
    rfbScreen->clientHead = NULL;
    INIT_MUTEX(rfbClientListMutex);
}

rfbClientIteratorPtr
rfbGetClientIterator(rfbScreenInfoPtr rfbScreen)
{
    rfbClientIteratorPtr i =
        (rfbClientIteratorPtr)MemAlloc(sizeof(struct rfbClientIterator));
    i->next = NULL;
    i->screen = rfbScreen;
    i->closedToo = FALSE;
    return i;
}

rfbClientIteratorPtr
rfbGetClientIteratorWithClosed(rfbScreenInfoPtr rfbScreen)
{
    rfbClientIteratorPtr i =
        (rfbClientIteratorPtr)MemAlloc(sizeof(struct rfbClientIterator));
    i->next = NULL;
    i->screen = rfbScreen;
    i->closedToo = TRUE;
    return i;
}

rfbClientPtr
rfbClientIteratorHead(rfbClientIteratorPtr i)
{
#ifdef LIBVNCSERVER_HAVE_LIBPTHREAD
    if(i->next != 0)
    {
        rfbDecrClientRef(i->next);
        rfbIncrClientRef(i->screen->clientHead);
    }
#endif
    LOCK(rfbClientListMutex);
    i->next = i->screen->clientHead;
    UNLOCK(rfbClientListMutex);
    return i->next;
}

rfbClientPtr
rfbClientIteratorNext(rfbClientIteratorPtr i)
{
    if(i->next == 0)
    {
        LOCK(rfbClientListMutex);
        i->next = i->screen->clientHead;
        UNLOCK(rfbClientListMutex);
    }
    else
    {
        IF_PTHREADS(rfbClientPtr cl = i->next);
        i->next = i->next->next;
        IF_PTHREADS(rfbDecrClientRef(cl));
    }

#ifdef LIBVNCSERVER_HAVE_LIBPTHREAD
    if(!i->closedToo)
        while(i->next && i->next->sock<0)
            i->next = i->next->next;
    if(i->next)
        rfbIncrClientRef(i->next);
#endif

    return i->next;
}

void
rfbReleaseClientIterator(rfbClientIteratorPtr iterator)
{
    IF_PTHREADS(if(iterator->next) rfbDecrClientRef(iterator->next));
    MemFree(iterator);
}


/*
 * rfbNewClientConnection is called from sockets.c when a new connection
 * comes in.
 */

void
rfbNewClientConnection(rfbScreenInfoPtr rfbScreen,
                       int sock)
{
    rfbClientPtr cl;

    cl = rfbNewClient(rfbScreen,sock);
#ifdef CORBA
    if(cl!=NULL)
        newConnection(cl, (KEYBOARD_DEVICE|POINTER_DEVICE),1,1,1);
#endif
}


/*
 * rfbReverseConnection is called to make an outward
 * connection to a "listening" RFB client.
 */

rfbClientPtr
rfbReverseConnection(rfbScreenInfoPtr rfbScreen,
                     char *host,
                     int port)
{
    int sock;
    rfbClientPtr cl;

    if ((sock = rfbConnect(rfbScreen, host, port)) < 0)
        return (rfbClientPtr)NULL;

    cl = rfbNewClient(rfbScreen, sock);

    if (cl)
    {
        cl->reverseConnection = TRUE;
    }

    return cl;
}


void
rfbSetProtocolVersion(rfbScreenInfoPtr rfbScreen, int major_, int minor_)
{
    /* Permit the server to set the version to report */
    /* TODO: sanity checking */
    if ((major_==3) && (minor_ > 2 && minor_ < 9))
    {
        rfbScreen->protocolMajorVersion = major_;
        rfbScreen->protocolMinorVersion = minor_;
    }
}

/*
 * rfbNewClient is called when a new connection has been made by whatever
 * means.
 */

static rfbClientPtr
rfbNewTCPOrUDPClient(rfbScreenInfoPtr rfbScreen,
                     int sock,
                     rfbBool isUDP)
{
    rfbProtocolVersionMsg pv;
    rfbClientIteratorPtr iterator;
    rfbClientPtr cl,cl_;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    rfbProtocolExtension* extension;

    cl = (rfbClientPtr)MemAlloc(sizeof(rfbClientRec));

    cl->screen = rfbScreen;
    cl->sock = sock;
    cl->viewOnly = FALSE;
    cl->allowClipboard = TRUE;
    /* setup pseudo scaling */
    cl->scaledScreen = rfbScreen;
    cl->scaledScreen->scaledScreenRefCount++;

    cl->clientData = NULL;
    cl->clientGoneHook = rfbDoNothingWithClient;

    if(!isUDP)
    {
        int one=1;

        getpeername(sock, (struct sockaddr *)&addr, &addrlen);
        cl->host = MemAlloc(16);
        mincpy(cl->host, inet_ntoa(addr.sin_addr), strlen(inet_ntoa(addr.sin_addr)));

        iterator = rfbGetClientIterator(rfbScreen);
        while ((cl_ = rfbClientIteratorNext(iterator)) != NULL)
        {
        }
        rfbReleaseClientIterator(iterator);

        if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                       (char *)&one, sizeof(one)) < 0)
        {
            closesocket(sock);
            return NULL;
        }

        FD_SET(sock,&(rfbScreen->allFds));
        rfbScreen->maxFd = max(sock,rfbScreen->maxFd);

        INIT_MUTEX(cl->outputMutex);
        INIT_MUTEX(cl->refCountMutex);
        INIT_MUTEX(cl->sendMutex);
        INIT_COND(cl->deleteCond);

        cl->state = RFB_PROTOCOL_VERSION;

        cl->reverseConnection = FALSE;
        cl->readyForSetColourMapEntries = FALSE;
        cl->useCopyRect = FALSE;
        cl->preferredEncoding = -1;
        cl->correMaxWidth = 48;
        cl->correMaxHeight = 48;
#ifdef LIBVNCSERVER_HAVE_LIBZ
        cl->zrleData = NULL;
#endif

        cl->copyRegion = sraRgnCreate();
        cl->copyDX = 0;
        cl->copyDY = 0;

        cl->modifiedRegion =
            sraRgnCreateRect(0,0,rfbScreen->width,rfbScreen->height);

        INIT_MUTEX(cl->updateMutex);
        INIT_COND(cl->updateCond);

        cl->requestedRegion = sraRgnCreate();

        cl->format = cl->screen->serverFormat;
        cl->translateFn = rfbTranslateNone;
        cl->translateLookupTable = NULL;

        LOCK(rfbClientListMutex);

        IF_PTHREADS(cl->refCount = 0);
        cl->next = rfbScreen->clientHead;
        cl->prev = NULL;
        if (rfbScreen->clientHead)
            rfbScreen->clientHead->prev = cl;

        rfbScreen->clientHead = cl;
        UNLOCK(rfbClientListMutex);

#ifdef LIBVNCSERVER_HAVE_LIBZ
        cl->tightQualityLevel = -1;
#ifdef LIBVNCSERVER_HAVE_LIBJPEG
        cl->tightCompressLevel = TIGHT_DEFAULT_COMPRESSION;
        {
            int i;
            for (i = 0; i < 4; i++)
                cl->zsActive[i] = FALSE;
        }
#endif
#endif

        cl->fileTransfer.fd_file = INVALID_HANDLE_VALUE;

        cl->enableCursorShapeUpdates = FALSE;
        cl->enableCursorPosUpdates = FALSE;
        cl->useRichCursorEncoding = FALSE;
        cl->enableLastRectEncoding = FALSE;
        cl->enableKeyboardLedState = FALSE;
        cl->enableSupportedMessages = FALSE;
        cl->enableSupportedEncodings = FALSE;
        cl->enableServerIdentity = FALSE;
        cl->lastKeyboardLedState = -1;
        cl->cursorX = rfbScreen->cursorX;
        cl->cursorY = rfbScreen->cursorY;
        cl->useNewFBSize = FALSE;

#ifdef LIBVNCSERVER_HAVE_LIBZ
        cl->compStreamInited = FALSE;
        cl->compStream.total_in = 0;
        cl->compStream.total_out = 0;
        cl->compStream.zalloc = Z_NULL;
        cl->compStream.zfree = Z_NULL;
        cl->compStream.opaque = Z_NULL;

        cl->zlibCompressLevel = 5;
#endif

        cl->progressiveSliceY = 0;

        cl->extensions = NULL;

        cl->lastPtrX = -1;

        //sprintf(pv,rfbProtocolVersionFormat,rfbScreen->protocolMajorVersion,
        //        rfbScreen->protocolMinorVersion);
        //
        // File transfer in UltraVNC works only when version = 3.4 or 3.6 or 3.14 or 3.16
        // Minor = 4 means that server supports FileTransfer and requires ms logon
        // Minor = 6 means that server support FileTransfer and requires normal VNC logon
        strcpy(pv, "RFB 003.006\n");

        if (rfbWriteExact(cl, pv, sz_rfbProtocolVersionMsg) < 0)
        {
            rfbCloseClient(cl);
            rfbClientConnectionGone(cl);
            return NULL;
        }
    }

    for(extension = rfbGetExtensionIterator(); extension;
            extension=extension->next)
    {
        void* data = NULL;
        /* if the extension does not have a newClient method, it wants
         * to be initialized later. */
        if(extension->newClient && extension->newClient(cl, &data))
            rfbEnableExtension(cl, extension, data);
    }
    rfbReleaseExtensionIterator();

    switch (cl->screen->newClientHook(cl))
    {
    case RFB_CLIENT_ON_HOLD:
        cl->onHold = TRUE;
        break;
    case RFB_CLIENT_ACCEPT:
        cl->onHold = FALSE;
        break;
    case RFB_CLIENT_REFUSE:
        rfbCloseClient(cl);
        rfbClientConnectionGone(cl);
        cl = NULL;
        break;
    }
    return cl;
}

rfbClientPtr
rfbNewClient(rfbScreenInfoPtr rfbScreen,
             int sock)
{
    return(rfbNewTCPOrUDPClient(rfbScreen,sock,FALSE));
}

rfbClientPtr
rfbNewUDPClient(rfbScreenInfoPtr rfbScreen)
{
    return((rfbScreen->udpClient=
                rfbNewTCPOrUDPClient(rfbScreen,rfbScreen->udpSock,TRUE)));
}

/*
 * rfbClientConnectionGone is called from sockets.c just after a connection
 * has gone away.
 */

void
rfbClientConnectionGone(rfbClientPtr cl)
{
#ifdef LIBVNCSERVER_HAVE_LIBJPEG
    int i;
#endif

    LOCK(rfbClientListMutex);

    if (cl->prev)
        cl->prev->next = cl->next;
    else
        cl->screen->clientHead = cl->next;
    if (cl->next)
        cl->next->prev = cl->prev;

    UNLOCK(rfbClientListMutex);

#ifdef LIBVNCSERVER_HAVE_LIBPTHREAD
    if(cl->screen->backgroundLoop != FALSE) {
      int i;
      do {
       LOCK(cl->refCountMutex);
       i=cl->refCount;
       if(i>0)
         WAIT(cl->deleteCond,cl->refCountMutex);
       UNLOCK(cl->refCountMutex);
      } while(i>0);
    }
#endif

    if(cl->sock != -1)
        closesocket(cl->sock);

    if (cl->scaledScreen!=NULL)
        cl->scaledScreen->scaledScreenRefCount--;

#ifdef LIBVNCSERVER_HAVE_LIBZ
    rfbFreeZrleData(cl);
#endif

    rfbFreeUltraData(cl);

    /* free buffers holding pixel data before and after encoding */
    MemFree(cl->beforeEncBuf);
    MemFree(cl->afterEncBuf);

    if(cl->sock>=0)
        FD_CLR(cl->sock,&(cl->screen->allFds));

    cl->clientGoneHook(cl);

    MemFree(cl->host);

#ifdef LIBVNCSERVER_HAVE_LIBZ
    /* Release the compression state structures if any. */
    if ( cl->compStreamInited )
    {
        deflateEnd( &(cl->compStream) );
    }

#ifdef LIBVNCSERVER_HAVE_LIBJPEG
    for (i = 0; i < 4; i++)
    {
        if (cl->zsActive[i])
            deflateEnd(&cl->zsStruct[i]);
    }
#endif
#endif

    if (cl->screen->pointerClient == cl)
        cl->screen->pointerClient = NULL;

    sraRgnDestroy(cl->modifiedRegion);
    sraRgnDestroy(cl->requestedRegion);
    sraRgnDestroy(cl->copyRegion);

    if (cl->translateLookupTable) MemFree(cl->translateLookupTable);

    TINI_COND(cl->updateCond);
    TINI_MUTEX(cl->updateMutex);

    /* make sure outputMutex is unlocked before destroying */
    LOCK(cl->outputMutex);
    UNLOCK(cl->outputMutex);
    TINI_MUTEX(cl->outputMutex);

#ifdef CORBA
    destroyConnection(cl);
#endif

    LOCK(cl->sendMutex);
    UNLOCK(cl->sendMutex);
    TINI_MUTEX(cl->sendMutex);

    MemFree(cl);
}


/*
 * rfbProcessClientMessage is called when there is data to read from a client.
 */

void
rfbProcessClientMessage(rfbClientPtr cl)
{
    switch (cl->state)
    {
    case RFB_PROTOCOL_VERSION:
        rfbProcessClientProtocolVersion(cl);
        return;
    case RFB_SECURITY_TYPE:
        rfbProcessClientSecurityType(cl);
        return;
    case RFB_AUTHENTICATION:
        rfbAuthProcessClientMessage(cl);
        return;
    case RFB_INITIALISATION:
        rfbProcessClientInitMessage(cl);
        return;
    default:
        rfbProcessClientNormalMessage(cl);
        return;
    }
}


/*
 * rfbProcessClientProtocolVersion is called when the client sends its
 * protocol version.
 */

static void
rfbProcessClientProtocolVersion(rfbClientPtr cl)
{
    rfbProtocolVersionMsg pv;
    int n, major_, minor_;

    if ((n = rfbReadExact(cl, pv, sz_rfbProtocolVersionMsg)) <= 0)
    {
        rfbCloseClient(cl);
        return;
    }

    pv[sz_rfbProtocolVersionMsg] = 0;
    if (sscanf(pv,rfbProtocolVersionFormat,&major_,&minor_) != 2)
    {
        rfbCloseClient(cl);
        return;
    }

    if (major_ != rfbProtocolMajorVersion)
    {
        rfbCloseClient(cl);
        return;
    }

    /* Check for the minor version use either of the two standard version of RFB */
    /*
     * UltraVNC Viewer detects FileTransfer compatible servers via rfb versions
     * 3.4, 3.6, 3.14, 3.16
     * It's a bad method, but it is what they use to enable features...
     * maintaining RFB version compatibility across multiple servers is a pain
     * Should use something like ServerIdentity encoding
     */
    cl->protocolMajorVersion = major_;
    cl->protocolMinorVersion = minor_;

    rfbAuthNewClient(cl);
}


void
rfbClientSendString(rfbClientPtr cl, char *reason)
{
    char *buf;
    int len = strlen(reason);

    buf = (char *)MemAlloc(4 + len);
    ((uint32_t *)buf)[0] = Swap32IfLE(len);
    memcpy(buf + 4, reason, len);

    rfbWriteExact(cl, buf, 4 + len);
    MemFree(buf);

    rfbCloseClient(cl);
}

/*
 * rfbClientConnFailed is called when a client connection has failed either
 * because it talks the wrong protocol or it has failed authentication.
 */

void
rfbClientConnFailed(rfbClientPtr cl,
                    char *reason)
{
    char *buf;
    int len = strlen(reason);

    buf = (char *)MemAlloc(8 + len);
    ((uint32_t *)buf)[0] = Swap32IfLE(rfbConnFailed);
    ((uint32_t *)buf)[1] = Swap32IfLE(len);
    memcpy(buf + 8, reason, len);

    rfbWriteExact(cl, buf, 8 + len);
    MemFree(buf);

    rfbCloseClient(cl);
}


/*
 * rfbProcessClientInitMessage is called when the client sends its
 * initialisation message.
 */

static void
rfbProcessClientInitMessage(rfbClientPtr cl)
{
    rfbClientInitMsg ci;
    union
    {
        char buf[256];
        rfbServerInitMsg si;
    } u;
    int len, n;
    rfbClientIteratorPtr iterator;
    rfbClientPtr otherCl;
    rfbExtensionData* extension;

    if ((n = rfbReadExact(cl, (char *)&ci,sz_rfbClientInitMsg)) <= 0)
    {
        rfbCloseClient(cl);
        return;
    }

    memset(u.buf,0,sizeof(u.buf));

    u.si.framebufferWidth = Swap16IfLE(cl->screen->width);
    u.si.framebufferHeight = Swap16IfLE(cl->screen->height);
    u.si.format = cl->screen->serverFormat;
    u.si.format.redMax = Swap16IfLE(u.si.format.redMax);
    u.si.format.greenMax = Swap16IfLE(u.si.format.greenMax);
    u.si.format.blueMax = Swap16IfLE(u.si.format.blueMax);

    strncpy(u.buf + sz_rfbServerInitMsg, cl->screen->desktopName, 127);
    len = strlen(u.buf + sz_rfbServerInitMsg);
    u.si.nameLength = Swap32IfLE(len);

    if (rfbWriteExact(cl, u.buf, sz_rfbServerInitMsg + len) < 0)
    {
        rfbCloseClient(cl);
        return;
    }

    for(extension = cl->extensions; extension;)
    {
        rfbExtensionData* next = extension->next;
        if(extension->extension->init &&
                !extension->extension->init(cl, extension->data))
            /* extension requested that it be removed */
            rfbDisableExtension(cl, extension->extension);
        extension = next;
    }

    cl->state = RFB_NORMAL;

    if (!cl->reverseConnection &&
            (cl->screen->neverShared || (!cl->screen->alwaysShared && !ci.shared)))
    {

        if (cl->screen->dontDisconnect)
        {
            iterator = rfbGetClientIterator(cl->screen);
            while ((otherCl = rfbClientIteratorNext(iterator)) != NULL)
            {
                if ((otherCl != cl) && (otherCl->state == RFB_NORMAL))
                {
                    rfbCloseClient(cl);
                    rfbReleaseClientIterator(iterator);
                    return;
                }
            }
            rfbReleaseClientIterator(iterator);
        }
        else
        {
            iterator = rfbGetClientIterator(cl->screen);
            while ((otherCl = rfbClientIteratorNext(iterator)) != NULL)
            {
                if ((otherCl != cl) && (otherCl->state == RFB_NORMAL))
                {
                    rfbCloseClient(otherCl);
                }
            }
            rfbReleaseClientIterator(iterator);
        }
    }
}

/* The values come in based on the scaled screen, we need to convert them to
 * values based on the man screen's coordinate system
 */
static rfbBool rectSwapIfLEAndClip(uint16_t* x,uint16_t* y,uint16_t* w,uint16_t* h,
                                   rfbClientPtr cl)
{
    int x1=Swap16IfLE(*x);
    int y1=Swap16IfLE(*y);
    int w1=Swap16IfLE(*w);
    int h1=Swap16IfLE(*h);

    rfbScaledCorrection(cl->scaledScreen, cl->screen, &x1, &y1, &w1, &h1);
    *x = x1;
    *y = y1;
    *w = w1;
    *h = h1;

    if(*w>cl->screen->width-*x)
        *w=cl->screen->width-*x;
    /* possible underflow */
    if(*w>cl->screen->width-*x)
        return FALSE;
    if(*h>cl->screen->height-*y)
        *h=cl->screen->height-*y;
    if(*h>cl->screen->height-*y)
        return FALSE;

    return TRUE;
}

/*
 * Send keyboard state (PointerPos pseudo-encoding).
 */

rfbBool
rfbSendKeyboardLedState(rfbClientPtr cl)
{
    rfbFramebufferUpdateRectHeader rect;

    if (cl->ublen + sz_rfbFramebufferUpdateRectHeader > UPDATE_BUF_SIZE)
    {
        if (!rfbSendUpdateBuf(cl))
            return FALSE;
    }

    rect.encoding = Swap32IfLE(rfbEncodingKeyboardLedState);
    rect.r.x = Swap16IfLE(cl->lastKeyboardLedState);
    rect.r.y = 0;
    rect.r.w = 0;
    rect.r.h = 0;

    memcpy(&cl->updateBuf[cl->ublen], (char *)&rect,
           sz_rfbFramebufferUpdateRectHeader);
    cl->ublen += sz_rfbFramebufferUpdateRectHeader;

    if (!rfbSendUpdateBuf(cl))
        return FALSE;

    return TRUE;
}


#define rfbSetBit(buffer, position)  (buffer[(position & 255) / 8] |= (1 << (position % 8)))

/*
 * Send rfbEncodingSupportedMessages.
 */

rfbBool
rfbSendSupportedMessages(rfbClientPtr cl)
{
    rfbFramebufferUpdateRectHeader rect;
    rfbSupportedMessages msgs;

    if (cl->ublen + sz_rfbFramebufferUpdateRectHeader
            + sz_rfbSupportedMessages > UPDATE_BUF_SIZE)
    {
        if (!rfbSendUpdateBuf(cl))
            return FALSE;
    }

    rect.encoding = Swap32IfLE(rfbEncodingSupportedMessages);
    rect.r.x = 0;
    rect.r.y = 0;
    rect.r.w = Swap16IfLE(sz_rfbSupportedMessages);
    rect.r.h = 0;

    memcpy(&cl->updateBuf[cl->ublen], (char *)&rect,
           sz_rfbFramebufferUpdateRectHeader);
    cl->ublen += sz_rfbFramebufferUpdateRectHeader;

    memset((char *)&msgs, 0, sz_rfbSupportedMessages);
    rfbSetBit(msgs.client2server, rfbSetPixelFormat);
    rfbSetBit(msgs.client2server, rfbFixColourMapEntries);
    rfbSetBit(msgs.client2server, rfbSetEncodings);
    rfbSetBit(msgs.client2server, rfbFramebufferUpdateRequest);
    rfbSetBit(msgs.client2server, rfbKeyEvent);
    rfbSetBit(msgs.client2server, rfbPointerEvent);
    rfbSetBit(msgs.client2server, rfbClientCutText);
    rfbSetBit(msgs.client2server, rfbFileTransfer);
    rfbSetBit(msgs.client2server, rfbSetScale);
    /*rfbSetBit(msgs.client2server, rfbSetServerInput);  */
    /*rfbSetBit(msgs.client2server, rfbSetSW);           */
    /*rfbSetBit(msgs.client2server, rfbTextChat);        */
    rfbSetBit(msgs.client2server, rfbPalmVNCSetScaleFactor);
    rfbSetBit(msgs.client2server, rfbXvp);

    rfbSetBit(msgs.server2client, rfbFramebufferUpdate);
    rfbSetBit(msgs.server2client, rfbSetColourMapEntries);
    rfbSetBit(msgs.server2client, rfbBell);
    rfbSetBit(msgs.server2client, rfbServerCutText);
    rfbSetBit(msgs.server2client, rfbResizeFrameBuffer);
    rfbSetBit(msgs.server2client, rfbPalmVNCReSizeFrameBuffer);
    rfbSetBit(msgs.server2client, rfbXvp);

    memcpy(&cl->updateBuf[cl->ublen], (char *)&msgs, sz_rfbSupportedMessages);
    cl->ublen += sz_rfbSupportedMessages;

    if (!rfbSendUpdateBuf(cl))
        return FALSE;

    return TRUE;
}



/*
 * Send rfbEncodingSupportedEncodings.
 */

rfbBool
rfbSendSupportedEncodings(rfbClientPtr cl)
{
    rfbFramebufferUpdateRectHeader rect;
    static uint32_t supported[] =
    {
        rfbEncodingRaw,
        rfbEncodingCopyRect,
        rfbEncodingRRE,
        rfbEncodingCoRRE,
        rfbEncodingHextile,
#ifdef LIBVNCSERVER_HAVE_LIBZ
        rfbEncodingZlib,
        rfbEncodingZRLE,
        rfbEncodingZYWRLE,
#endif
#ifdef LIBVNCSERVER_HAVE_LIBJPEG
        rfbEncodingTight,
#endif
        rfbEncodingUltra,
        rfbEncodingUltraZip,
        rfbEncodingXCursor,
        rfbEncodingRichCursor,
        rfbEncodingPointerPos,
        rfbEncodingLastRect,
        rfbEncodingNewFBSize,
        rfbEncodingKeyboardLedState,
        rfbEncodingSupportedMessages,
        rfbEncodingSupportedEncodings,
        rfbEncodingServerIdentity,
    };
    uint32_t nEncodings = sizeof(supported) / sizeof(supported[0]), i;

    /* think rfbSetEncodingsMsg */

    if (cl->ublen + sz_rfbFramebufferUpdateRectHeader
            + (nEncodings * sizeof(uint32_t)) > UPDATE_BUF_SIZE)
    {
        if (!rfbSendUpdateBuf(cl))
            return FALSE;
    }

    rect.encoding = Swap32IfLE(rfbEncodingSupportedEncodings);
    rect.r.x = 0;
    rect.r.y = 0;
    rect.r.w = Swap16IfLE(nEncodings * sizeof(uint32_t));
    rect.r.h = Swap16IfLE(nEncodings);

    memcpy(&cl->updateBuf[cl->ublen], (char *)&rect,
           sz_rfbFramebufferUpdateRectHeader);
    cl->ublen += sz_rfbFramebufferUpdateRectHeader;

    for (i = 0; i < nEncodings; i++)
    {
        uint32_t encoding = Swap32IfLE(supported[i]);
        memcpy(&cl->updateBuf[cl->ublen], (char *)&encoding, sizeof(encoding));
        cl->ublen += sizeof(encoding);
    }

    if (!rfbSendUpdateBuf(cl))
        return FALSE;

    return TRUE;
}


void
rfbSetServerVersionIdentity(rfbScreenInfoPtr screen, char *fmt, ...)
{
    char buffer[256];
    va_list ap;

    va_start(ap, fmt);
    wvnsprintf(buffer, sizeof(buffer)-1, fmt, ap);
    va_end(ap);

    if (screen->versionString!=NULL) MemFree(screen->versionString);
    screen->versionString = MemAlloc(strlen(buffer));
    mincpy(screen->versionString, buffer, strlen(buffer));
}

/*
 * Send rfbEncodingServerIdentity.
 */

rfbBool
rfbSendServerIdentity(rfbClientPtr cl)
{
    rfbFramebufferUpdateRectHeader rect;
    char buffer[512];

    /* tack on our library version */
    wsprintf(buffer, "%s (%s)",
             (cl->screen->versionString==NULL ? "unknown" : cl->screen->versionString),
             LIBVNCSERVER_PACKAGE_STRING);

    if (cl->ublen + sz_rfbFramebufferUpdateRectHeader
            + (strlen(buffer)+1) > UPDATE_BUF_SIZE)
    {
        if (!rfbSendUpdateBuf(cl))
            return FALSE;
    }

    rect.encoding = Swap32IfLE(rfbEncodingServerIdentity);
    rect.r.x = 0;
    rect.r.y = 0;
    rect.r.w = Swap16IfLE(strlen(buffer)+1);
    rect.r.h = 0;

    memcpy(&cl->updateBuf[cl->ublen], (char *)&rect,
           sz_rfbFramebufferUpdateRectHeader);
    cl->ublen += sz_rfbFramebufferUpdateRectHeader;

    memcpy(&cl->updateBuf[cl->ublen], buffer, strlen(buffer)+1);
    cl->ublen += strlen(buffer)+1;

    if (!rfbSendUpdateBuf(cl))
        return FALSE;

    return TRUE;
}

/*
 * Send an xvp server message
 */

rfbBool
rfbSendXvp(rfbClientPtr cl, uint8_t version, uint8_t code)
{
    rfbXvpMsg xvp;

    xvp.type = rfbXvp;
    xvp.pad = 0;
    xvp.version = version;
    xvp.code = code;

    LOCK(cl->sendMutex);
    if (rfbWriteExact(cl, (char *)&xvp, sz_rfbXvpMsg) < 0)
    {
        rfbCloseClient(cl);
    }
    UNLOCK(cl->sendMutex);

    return TRUE;
}


rfbBool rfbSendTextChatMessage(rfbClientPtr cl, uint32_t length, char *buffer)
{
    rfbTextChatMsg tc;
    int bytesToSend=0;

    memset((char *)&tc, 0, sizeof(tc));
    tc.type = rfbTextChat;
    tc.length = Swap32IfLE(length);

    switch(length)
    {
    case rfbTextChatOpen:
    case rfbTextChatClose:
    case rfbTextChatFinished:
        bytesToSend=0;
        break;
    default:
        bytesToSend=length;
        if (bytesToSend>rfbTextMaxSize)
            bytesToSend=rfbTextMaxSize;
    }

    if (cl->ublen + sz_rfbTextChatMsg + bytesToSend > UPDATE_BUF_SIZE)
    {
        if (!rfbSendUpdateBuf(cl))
            return FALSE;
    }

    memcpy(&cl->updateBuf[cl->ublen], (char *)&tc, sz_rfbTextChatMsg);
    cl->ublen += sz_rfbTextChatMsg;
    if (bytesToSend>0)
    {
        memcpy(&cl->updateBuf[cl->ublen], buffer, bytesToSend);
        cl->ublen += bytesToSend;
    }

    if (!rfbSendUpdateBuf(cl))
        return FALSE;

    return TRUE;
}

#define FILEXFER_ALLOWED_OR_CLOSE_AND_RETURN(msg, cl, ret) \
	if ((cl->screen->getFileTransferPermission != NULL \
	    && cl->screen->getFileTransferPermission(cl) != TRUE)) { \
		rfbCloseClient(cl); \
		return ret; \
	}

int DB = 1;

rfbBool rfbSendFileTransferMessage(rfbClientPtr cl, uint8_t contentType, uint8_t contentParam, uint32_t size, uint32_t length, char *buffer)
{
    rfbFileTransferMsg ft;
    ft.type = rfbFileTransfer;
    ft.contentType = contentType;
    ft.contentParam = contentParam;
    ft.pad          = 0; /* UltraVNC did not Swap16LE(ft.contentParam) (Looks like it might be BigEndian) */
    ft.size         = Swap32IfLE(size);
    ft.length       = Swap32IfLE(length);

    FILEXFER_ALLOWED_OR_CLOSE_AND_RETURN("", cl, FALSE);
    LOCK(cl->sendMutex);
    if (rfbWriteExact(cl, (char *)&ft, sz_rfbFileTransferMsg) < 0)
    {
        rfbCloseClient(cl);
        UNLOCK(cl->sendMutex);
        return FALSE;
    }

    if (length>0)
    {
        if (rfbWriteExact(cl, buffer, length) < 0)
        {
            rfbCloseClient(cl);
            UNLOCK(cl->sendMutex);
            return FALSE;
        }
    }
    UNLOCK(cl->sendMutex);

    return TRUE;
}

/*
 * UltraVNC uses Windows Structures
 */
#define MAX_PATH 260
// for bellow stuff we use real windows stuff
/*
typedef struct {
    uint32_t dwLowDateTime;
    uint32_t dwHighDateTime;
} RFB_FILETIME;

typedef struct {
    uint32_t dwFileAttributes;
    RFB_FILETIME ftCreationTime;
    RFB_FILETIME ftLastAccessTime;
    RFB_FILETIME ftLastWriteTime;
    uint32_t nFileSizeHigh;
    uint32_t nFileSizeLow;
    uint32_t dwReserved0;
    uint32_t dwReserved1;
    uint8_t  cFileName[ MAX_PATH ];
    uint8_t  cAlternateFileName[ 14 ];
} RFB_FIND_DATA;

#define RFB_FILE_ATTRIBUTE_READONLY   0x1
#define RFB_FILE_ATTRIBUTE_HIDDEN     0x2
#define RFB_FILE_ATTRIBUTE_SYSTEM     0x4
#define RFB_FILE_ATTRIBUTE_DIRECTORY  0x10
#define RFB_FILE_ATTRIBUTE_ARCHIVE    0x20
#define RFB_FILE_ATTRIBUTE_NORMAL     0x80
#define RFB_FILE_ATTRIBUTE_TEMPORARY  0x100
#define RFB_FILE_ATTRIBUTE_COMPRESSED 0x800
*/

#define rfbFilenameTranslate2UNIX(not_used, src, dest) strcpy(dest, src)
/*rfbBool rfbFilenameTranslate2UNIX(rfbClientPtr cl, char *path, char *unixPath)
{
    int x;
    char *home=NULL;

    FILEXFER_ALLOWED_OR_CLOSE_AND_RETURN("", cl, FALSE);
*/
/* C: */
/*    if (path[1]==':')
      strcpy(unixPath, &path[2]);

    for (x=0;x<(int)strlen(unixPath);x++)
      if (unixPath[x]=='\\') unixPath[x]='/';
    return TRUE;
}*/

rfbBool rfbFilenameTranslate2DOS(rfbClientPtr cl, char *unixPath, char *path)
{
    int x;

    FILEXFER_ALLOWED_OR_CLOSE_AND_RETURN("", cl, FALSE);

    //sprintf(path,"C:%s", unixPath);
    strcpy(path, "C:");
    strcat(path, unixPath);
    for (x=2; x<(int)strlen(path); x++)
        if (path[x]=='/') path[x]='\\';
    return TRUE;
}

rfbBool rfbSendDirContent(rfbClientPtr cl, int length, char *buffer)
{
    char path[MAX_PATH];
    WCHAR pathW[MAX_PATH];
    WIN32_FIND_DATAA win32filenameA = {0};
    WIN32_FIND_DATAW win32filenameW = {0};
    int nOptLen = 0, retval=0, nFolder = -1,dwLen;
    HANDLE hFindHandle;

    FILEXFER_ALLOWED_OR_CLOSE_AND_RETURN("", cl, FALSE);

    // Client thinks we are Winblows... we are...
    rfbFilenameTranslate2UNIX(cl, buffer, path);

    if (!memcmp(path, "My Documents", 12))
        nFolder = CSIDL_PERSONAL;
    if (!memcmp(path, "Desktop", 7))
        nFolder = CSIDL_DESKTOP;
    if (!memcmp(path, "Network Favorites", 17))
        nFolder = CSIDL_NETHOOD;

    if (nFolder != -1)
    {
        char tmp[MAX_PATH],*p;
        SHGetSpecialFolderPath(NULL, tmp, nFolder, FALSE);
        p=strchr(path,'\\');
        if (p && *(p+1))
        {
            strcat(tmp,p);
        }
        strcpy(path,tmp);
    }

    if(path[strlen(path) - 1] == '\\')
        strcat(path, "*");
    else
        strcat(path, "\\*");

    dwLen=strlen(path)+1;
    MultiByteToWideChar(1251,0,path,dwLen,pathW,dwLen);

    hFindHandle = FindFirstFileW(pathW, &win32filenameW);
    if(hFindHandle == INVALID_HANDLE_VALUE)
        return rfbSendFileTransferMessage(cl, rfbDirPacket, rfbADirectory, 0, 0, NULL);

    // send back the path name (necessary for links)
    if (rfbSendFileTransferMessage(cl, rfbDirPacket, rfbADirectory, 0, length, buffer)==FALSE) return FALSE;
    do
    {
        // normally everything goes in big endian, but this struct should be little?

        // Do not show hidden files (but show how to move up the tree)
        if (wcscmp(win32filenameW.cFileName, L"."))
        {
            memcpy(&win32filenameA,&win32filenameW,((int)&win32filenameA.cFileName)-(int)&win32filenameA);

            dwLen=lstrlenW(win32filenameW.cFileName)+1;
            WideCharToMultiByte(1251,WC_COMPOSITECHECK,win32filenameW.cFileName,dwLen,win32filenameA.cFileName,dwLen,NULL,NULL);
            dwLen=lstrlenW(win32filenameW.cAlternateFileName);
            if (dwLen)
                WideCharToMultiByte(1251,WC_COMPOSITECHECK,win32filenameW.cAlternateFileName,dwLen,win32filenameA.cAlternateFileName,dwLen,NULL,NULL);

            nOptLen = sizeof(WIN32_FIND_DATAA) - MAX_PATH - 14 + strlen((char *)win32filenameA.cFileName);

            if (rfbSendFileTransferMessage(cl, rfbDirPacket, rfbADirectory, 0, nOptLen, (char *)&win32filenameA)==FALSE) return FALSE;
        }

    }
    while(FindNextFileW(hFindHandle, &win32filenameW));
    FindClose(hFindHandle);
    // End of the transfer
    return rfbSendFileTransferMessage(cl, rfbDirPacket, 0, 0, 0, NULL);
}


char *rfbProcessFileTransferReadBuffer(rfbClientPtr cl, uint32_t length)
{
    char *buffer=NULL;
    int   n=0;

    FILEXFER_ALLOWED_OR_CLOSE_AND_RETURN("", cl, NULL);
    /*
    rfbLog("rfbProcessFileTransferReadBuffer(%dlen)\n", length);
    */
    if (length>0)
    {
        buffer=MemAlloc(length+1);
        if (buffer!=NULL)
        {
            if ((n = rfbReadExact(cl, (char *)buffer, length)) <= 0)
            {
                rfbCloseClient(cl);
                /* NOTE: don't forget to free(buffer) if you return early! */
                if (buffer!=NULL) MemFree(buffer);
                return NULL;
            }
            /* Null Terminate */
            buffer[length]=0;
        }
    }
    return buffer;
}


rfbBool rfbSendFileTransferChunk(rfbClientPtr cl)
{
    /* Allocate buffer for compression */
    unsigned char readBuf[sz_rfbBlockSize];
    int bytesRead=0;
    int retval=0;
    fd_set wfds;
    struct timeval tv;
    int n;
    int i;
#ifdef LIBVNCSERVER_HAVE_LIBZ
    unsigned char compBuf[sz_rfbBlockSize + 1024];
    unsigned long nMaxCompSize = sizeof(compBuf);
    int nRetC = 0;
#endif

    /*
     * Don't close the client if we get into this one because
     * it is called from many places to service file transfers.
     * Note that permitFileTransfer is checked first.
     */
    if (cl->screen->getFileTransferPermission != NULL
        && cl->screen->getFileTransferPermission(cl) != TRUE)
    {
        return TRUE;
    }

    // If not sending, or no file _open...   Return as if we sent something!
    //if ((cl->fileTransfer.fd!=-1) && (cl->fileTransfer.sending==1))
    if ((cl->fileTransfer.fd_file!=INVALID_HANDLE_VALUE) && (cl->fileTransfer.sending==1))
    {
        FD_ZERO(&wfds);
        FD_SET(cl->sock, &wfds);

        // return immediately
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        n = select(cl->sock + 1, NULL, &wfds, NULL, &tv);

        // We have space on the transmit queue
        if (n > 0)
        {
            for(i = 0; i < 32; i++)	// 32 * 8 = 256kb
            {
                if(!ReadFile(cl->fileTransfer.fd_file, readBuf, sz_rfbBlockSize, (LPDWORD)&bytesRead, NULL))
                    bytesRead = -1;	// indicate error
                switch (bytesRead)
                {
                case 0:
                {
                    retval = rfbSendFileTransferMessage(cl, rfbEndOfFile, 0, 0, 0, NULL);

                    SysCloseHandle(cl->fileTransfer.fd_file);
                    cl->fileTransfer.fd_file = INVALID_HANDLE_VALUE;
                    cl->fileTransfer.sending   = 0;
                    cl->fileTransfer.receiving = 0;
                    if (cl->fileTransfer.bDir)
                        DeleteFileW(cl->fileTransfer.szCabFile);
                    return retval;
                }
                case -1:
                {
                    // TODO : send an error msg to the client...
                    retval = rfbSendFileTransferMessage(cl, rfbAbortFileTransfer, 0, 0, 0, NULL);

                    SysCloseHandle(cl->fileTransfer.fd_file);
                    cl->fileTransfer.fd_file = INVALID_HANDLE_VALUE;
                    cl->fileTransfer.sending   = 0;
                    cl->fileTransfer.receiving = 0;
                    if (cl->fileTransfer.bDir)
                        DeleteFileW(cl->fileTransfer.szCabFile);
                    return retval;
                }
                default:
                {
                    if (!cl->fileTransfer.compressionEnabled)
                        retval = rfbSendFileTransferMessage(cl, rfbFilePacket, 0, 0, bytesRead, readBuf);
                    else
                    {
#ifdef LIBVNCSERVER_HAVE_LIBZ
                        nRetC = compress(compBuf, &nMaxCompSize, readBuf, bytesRead);

                        if ((nRetC==0) && (nMaxCompSize<bytesRead))
                            return  rfbSendFileTransferMessage(cl, rfbFilePacket, 0, 1, nMaxCompSize, (char *)compBuf);
                        else
                            return  rfbSendFileTransferMessage(cl, rfbFilePacket, 0, 0, bytesRead, (char *)readBuf);
#else
                        /* We do not support compression of the data stream */
                        return  rfbSendFileTransferMessage(cl, rfbFilePacket, 0, 0, bytesRead, (char *)readBuf);
#endif
                    }
                }
                }
            }
        }
    }
    return TRUE;
}

WCHAR *GetFileName(WCHAR *lpPath)
{
    WCHAR *p=lpPath+lstrlenW(lpPath);
    for (; *(p-1) != L'\\'; p--) ;
    return p;
}

rfbBool rfbProcessFileTransfer(rfbClientPtr cl, uint8_t contentType, uint8_t contentParam, uint32_t size, uint32_t length)
{
    char *buffer=NULL, *p=NULL;
    int retval=0;
    char filename1[MAX_PATH];
    char filename2[MAX_PATH];
    char szFileTime[MAX_PATH];
    WCHAR filename1W[MAX_PATH];
    WCHAR filename2W[MAX_PATH];
    //struct stat statbuf;
    BY_HANDLE_FILE_INFORMATION fstats;
    DWORD dwFileAttributes;
    SYSTEMTIME st;
    uint32_t sizeHtmp=0;
    int n=0;
    char timespec[64];
#ifdef LIBVNCSERVER_HAVE_LIBZ
    unsigned char compBuff[sz_rfbBlockSize];
    unsigned long nRawBytes = sz_rfbBlockSize;
    int nRet = 0;
#endif
    TCHAR szDrivesList[256]; // Format when filled : "C:\<NULL>D:\<NULL>....Z:\<NULL><NULL>
    DWORD dwLen, nFolder = -1;
    int nIndex = 0;
    int nType = 0;
    TCHAR szDrive[4];

    FILEXFER_ALLOWED_OR_CLOSE_AND_RETURN("", cl, FALSE);

    switch (contentType)
    {
    case rfbDirContentRequest:
        switch (contentParam)
        {
        case rfbRDrivesList: // Client requests the List of Local Drives

            //rfbLog("rfbProcessFileTransfer() rfbDirContentRequest: rfbRDrivesList:\n");

            // Format when filled : "C:\<NULL>D:\<NULL>....Z:\<NULL><NULL>
            //
            // We replace the "\" char following the drive letter and ":"
            // with a char corresponding to the type of drive
            // We obtain something like "C:l<NULL>D:c<NULL>....Z:n\<NULL><NULL>"
            //  Isn't it ugly ?
            // DRIVE_FIXED = 'l'     (local?)
            // DRIVE_REMOVABLE = 'f' (floppy?)
            // DRIVE_CDROM = 'c'
            // DRIVE_REMOTE = 'n'
            //

            // in unix, there are no 'drives'  (We could list mount points though)
            // We fake the root as a "C:" for the Winblows users

            dwLen = GetLogicalDriveStrings(256, szDrivesList);

            while (nIndex < dwLen - 3)
            {
                strcpy(szDrive, szDrivesList + nIndex);
                // We replace the "\" char following the drive letter and ":"
                // with a char corresponding to the type of drive
                // We obtain something like "C:l<NULL>D:c<NULL>....Z:n\<NULL><NULL>"
                // Isn't it ugly ?
                nType = GetDriveType(szDrive);
                switch (nType)
                {
                case DRIVE_FIXED:
                    szDrivesList[nIndex + 2] = 'l';
                    break;
                case DRIVE_REMOVABLE:
                    szDrivesList[nIndex + 2] = 'f';
                    break;
                case DRIVE_CDROM:
                    szDrivesList[nIndex + 2] = 'c';
                    break;
                case DRIVE_REMOTE:
                    szDrivesList[nIndex + 2] = 'n';
                    break;
                }
                nIndex += 4;
            }

            retval = rfbSendFileTransferMessage(cl, rfbDirPacket, rfbADrivesList, 0, dwLen, szDrivesList);
            if (buffer!=NULL) MemFree(buffer);
            return retval;
            break;
        case rfbRDirContent: // Client requests the content of a directory

            //rfbLog("rfbProcessFileTransfer() rfbDirContentRequest: rfbRDirContent\n");

            SetErrorMode(SEM_FAILCRITICALERRORS);

            if ((buffer = rfbProcessFileTransferReadBuffer(cl, length))==NULL) return FALSE;
            retval = rfbSendDirContent(cl, length, buffer);
            if (buffer!=NULL) MemFree(buffer);
            return retval;
        }
        break;
    case rfbFileTransferRequest:

        //rfbLog("rfbProcessFileTransfer() rfbFileTransferRequest:\n");

        // add some space to the end of the buffer as we will be adding a timespec to it
        if ((buffer = rfbProcessFileTransferReadBuffer(cl, length))==NULL) return FALSE;
        // The client requests a File
        rfbFilenameTranslate2UNIX(cl, buffer, filename1);

        if (!memcmp(filename1, "My Documents", 12))
            nFolder = CSIDL_PERSONAL;
        if (!memcmp(filename1, "Desktop", 7))
            nFolder = CSIDL_DESKTOP;
        if (!memcmp(filename1, "Network Favorites", 17))
            nFolder = CSIDL_NETHOOD;

        if (nFolder != -1)
        {
            char tmp[MAX_PATH],*p;
            SHGetSpecialFolderPath(NULL, tmp, nFolder, FALSE);
            p=strchr(filename1,'\\');
            if (p && *(p+1))
                strcat(tmp,p);
            strcpy(filename1,tmp);
        }

        dwLen=strlen(filename1)+1;
        MultiByteToWideChar(1251,0,filename1,dwLen,filename1W,dwLen);
        if (GetFileAttributesW(filename1W) == INVALID_FILE_ATTRIBUTES)
        {
            WCHAR *p=GetFileName(filename1W);

            if (*p == '[')
            {
                DWORD dwAttribs=0;
                int dwLen=p-filename1W;

                memcpy(filename2W,filename1W,dwLen*sizeof(WCHAR));
                filename2W[dwLen]=0;
                lstrcatW(filename2W,p+2);
                filename2W[lstrlenW(filename2W)-2]=0;

                if (((dwAttribs=GetFileAttributesW(filename2W)) != INVALID_FILE_ATTRIBUTES) && (dwAttribs & FILE_ATTRIBUTE_DIRECTORY))
                {
                    CHAR *p=0;
                    WCHAR szmask[4]=L"*.*";
                    WCHAR *lpmask[2]= {szmask,0};
                    WCHAR tmppath[MAX_PATH];

                    GetTempPathW(MAX_PATH-1,tmppath);
                    GetTempFileNameW(tmppath,L"_",0,filename1W);
                    DeleteFileW(filename1W);

                    ArchCreateFromFolderW(filename1W,filename2W,lpmask,1,CFF_RECURSE,0,0,5);
                    cl->fileTransfer.bDir=TRUE;
                    lstrcpyW(cl->fileTransfer.szCabFile,filename1W);

                    p=filename1+dwLen;
                    memcpy(filename2,filename1,dwLen);
                    filename2[dwLen]=0;
                    lstrcat(filename2,p+2);
                    filename2[lstrlen(filename2)-2]=0;
                    buffer=MemRealloc(buffer,strlen(filename2)+5);
                    lstrcpy(buffer,filename2);
                    lstrcat(buffer,".zip");
                }
            }
        }

        cl->fileTransfer.fd_file = CreateFileW(filename1W, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

        if (cl->fileTransfer.fd_file!=INVALID_HANDLE_VALUE)
        {
            if (GetFileInformationByHandle(cl->fileTransfer.fd_file, &fstats))
            {
                // Add the File Time Stamp to the filename
                FileTimeToSystemTime(&fstats.ftCreationTime, &st);
                wsprintfA(timespec, "%02d/%02d/%04d %02d:%02d",
                          st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute);
                buffer=MemRealloc(buffer, lstrlen(buffer) + strlen(timespec) + 2); // comma, and Null term
                if (buffer==NULL)
                {
                    return FALSE;
                }
                strcat(buffer,",");
                strcat(buffer, timespec);
                length = lstrlen(buffer);
            }
            else
            {
                SysCloseHandle(cl->fileTransfer.fd_file);
                cl->fileTransfer.fd_file = INVALID_HANDLE_VALUE;
            }
        }
        //    if (cl->fileTransfer.fd!=-1) {
        //        if (fstat(cl->fileTransfer.fd, &statbuf)!=0) {
        //			fclose((FILE*)cl->fileTransfer.fd);
        //            cl->fileTransfer.fd=-1;
        //        }
        //        else
        //        {
        //          // Add the File Time Stamp to the filename
        //          strftime(timespec, sizeof(timespec), "%m/%d/%Y %H:%M",gmtime(&statbuf.st_ctime));
        //          buffer=realloc(buffer, length + strlen(timespec) + 2); // comma, and Null term
        //          if (buffer==NULL) {
        //              return FALSE;
        //          }
        //          strcat(buffer,",");
        //          strcat(buffer, timespec);
        //          length = strlen(buffer);
        //        }
        //    }

        // The viewer supports compression if size==1
        cl->fileTransfer.compressionEnabled = (size==1);


        //rfbLog("rfbProcessFileTransfer() rfbFileTransferRequest(\"%s\"->\"%s\")%s\n", buffer, filename1, (size==1?" <Compression Enabled>":""));


        // File Size in bytes, 0xFFFFFFFF (-1) means error
        retval = rfbSendFileTransferMessage(cl, rfbFileHeader, 0, (cl->fileTransfer.fd_file==INVALID_HANDLE_VALUE ? -1 : fstats.nFileSizeLow), length, buffer);
        //retval = rfbSendFileTransferMessage(cl, rfbFileHeader, 0, (cl->fileTransfer.fd==-1 ? -1 : statbuf.st_size), length, buffer);

        //if (cl->fileTransfer.fd==-1)
        if (cl->fileTransfer.fd_file==INVALID_HANDLE_VALUE)
        {
            if (buffer!=NULL) MemFree(buffer);
            return retval;
        }
        // setup filetransfer stuff
        //cl->fileTransfer.fileSize = statbuf.st_size;
        //cl->fileTransfer.numPackets = statbuf.st_size / sz_rfbBlockSize;
        cl->fileTransfer.fileSize = fstats.nFileSizeLow;
        cl->fileTransfer.numPackets = fstats.nFileSizeLow / sz_rfbBlockSize;
        cl->fileTransfer.receiving = 0;
        cl->fileTransfer.sending = 0; /* set when we receive a rfbFileHeader: */

        // TODO: finish 64-bit file size support
        sizeHtmp = 0;
        LOCK(cl->sendMutex);
        //sizeHtmp = fstats.nFileSizeHigh; ?
        if (rfbWriteExact(cl, (char *)&sizeHtmp, 4) < 0)
        {
            if (cl->fileTransfer.bDir)
                DeleteFile(filename1);
            // shouldnt we close file here?
            rfbCloseClient(cl);
            UNLOCK(cl->sendMutex);
            if (buffer!=NULL) MemFree(buffer);
            return FALSE;
        }
        UNLOCK(cl->sendMutex);
        break;

    case rfbFileHeader:
        // Destination file (viewer side) is ready for reception (size > 0) or not (size = -1)
        if (size==-1)
        {
            //closesocket(cl->fileTransfer.fd);
            //cl->fileTransfer.fd=-1;
            // why close socket when it's file?
            SysCloseHandle(cl->fileTransfer.fd_file);
            cl->fileTransfer.fd_file = INVALID_HANDLE_VALUE;
            return TRUE;
        }


        //rfbLog("rfbProcessFileTransfer() rfbFileHeader (%d bytes of a file)\n", size);


        // Starts the transfer!
        cl->fileTransfer.sending=1;
        return rfbSendFileTransferChunk(cl);
        break;


        // sending from the viewer to the server


    case rfbFileTransferOffer:
        // client is sending a file to us
        // buffer contains full path name (plus FileTime)
        // size contains size of the file
        //
        //rfbLog("rfbProcessFileTransfer() rfbFileTransferOffer:\n";

        if ((buffer = rfbProcessFileTransferReadBuffer(cl, length))==NULL) return FALSE;

        // Parse the FileTime
        //p = strrchr(buffer, ',');
        // i think above is same as p = strchr(...), but that still needs crt,
        // and doesnt look like msvc has intrinsic for scasb, so...
        p = buffer;
        while(*p && *p != ',') p++;
        if(*p == 0) p = NULL;

        if (p!=NULL)
        {
            *p = '\0';
            strcpy(szFileTime, p+1);
        }
        else
            szFileTime[0]=0;



        // Need to _read in sizeHtmp
        if ((n = rfbReadExact(cl, (char *)&sizeHtmp, 4)) <= 0)
        {
            rfbCloseClient(cl);
            // NOTE: don't forget to free(buffer) if you return early!
            if (buffer!=NULL) MemFree(buffer);
            return FALSE;
        }
        sizeHtmp = Swap32IfLE(sizeHtmp);

        rfbFilenameTranslate2UNIX(cl, buffer, filename1);

        // If the file exists... We can send a rfbFileChecksums back to the client before we send an rfbFileAcceptHeader
        // TODO: Delta Transfer

        //cl->fileTransfer.fd=_open(filename1, O_CREAT|O_WRONLY|O_TRUNC, 0744);
        cl->fileTransfer.fd_file = CreateFileA(filename1, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

        // File Size in bytes, 0xFFFFFFFF (-1) means error
        retval = rfbSendFileTransferMessage(cl, rfbFileAcceptHeader, 0, (cl->fileTransfer.fd_file==INVALID_HANDLE_VALUE ? -1 : 0), length, buffer);
        //retval = rfbSendFileTransferMessage(cl, rfbFileAcceptHeader, 0, (cl->fileTransfer.fd==-1 ? -1 : 0), length, buffer);
        //if (cl->fileTransfer.fd==-1) {
        if (cl->fileTransfer.fd_file==INVALID_HANDLE_VALUE)
        {
            MemFree(buffer);
            return retval;
        }

        // setup filetransfer stuff
        cl->fileTransfer.fileSize = size;
        cl->fileTransfer.numPackets = size / sz_rfbBlockSize;
        cl->fileTransfer.receiving = 1;
        cl->fileTransfer.sending = 0;
        break;

    case rfbFilePacket:

        //rfbLog("rfbProcessFileTransfer() rfbFilePacket:\n");

        if ((buffer = rfbProcessFileTransferReadBuffer(cl, length))==NULL) return FALSE;
        if (cl->fileTransfer.fd_file!=INVALID_HANDLE_VALUE)
        {
            //if (cl->fileTransfer.fd!=-1) {
            // buffer contains the contents of the file
            if (size==0)
                // why x_write is defined as send?
                //retval=x_write(cl->fileTransfer.fd, buffer, length);
                WriteFile(cl->fileTransfer.fd_file, buffer, length, (LPDWORD)&retval, NULL);
            else
            {
#ifdef LIBVNCSERVER_HAVE_LIBZ
                // compressed packet
                nRet = uncompress(compBuff,&nRawBytes,(const unsigned char*)buffer, length);
                //retval=x_write(cl->fileTransfer.fd, compBuff, nRawBytes);
                WriteFile(cl->fileTransfer.fd_file, compBuff, nRawBytes, (LPDWORD)&retval, NULL);
#else
                // Write the file out as received...
                //retval=x_write(cl->fileTransfer.fd, buffer, length);
                WriteFile(cl->fileTransfer.fd_file, buffer, length, (LPDWORD)&retval, NULL);
#endif
            }
            if (retval==-1)
            {
                //closesocket(cl->fileTransfer.fd);
                //cl->fileTransfer.fd=-1;
                // wth why sockets?
                SysCloseHandle(cl->fileTransfer.fd_file);
                cl->fileTransfer.fd_file = INVALID_HANDLE_VALUE;
                cl->fileTransfer.sending   = 0;
                cl->fileTransfer.receiving = 0;
            }
        }
        break;

    case rfbEndOfFile:
        //if (cl->fileTransfer.fd!=-1)
        //    closesocket(cl->fileTransfer.fd);
        //cl->fileTransfer.fd=-1;
        if (cl->fileTransfer.fd_file!=INVALID_HANDLE_VALUE)
            SysCloseHandle(cl->fileTransfer.fd_file);
        cl->fileTransfer.fd_file=INVALID_HANDLE_VALUE;
        cl->fileTransfer.sending   = 0;
        cl->fileTransfer.receiving = 0;
        break;

    case rfbAbortFileTransfer:
        if (cl->fileTransfer.fd_file!=INVALID_HANDLE_VALUE)
        {
            SysCloseHandle(cl->fileTransfer.fd_file);
            cl->fileTransfer.fd_file=INVALID_HANDLE_VALUE;
            cl->fileTransfer.sending   = 0;
            cl->fileTransfer.receiving = 0;
        }
        else
        {
            // We use this message for FileTransfer rights (<=RC18 versions)
            // The client asks for FileTransfer permission
            //
            if (contentParam == 0)
            {
                // Old method for FileTransfer handshake perimssion (<=RC18) (Deny it)
                return rfbSendFileTransferMessage(cl, rfbAbortFileTransfer, 0, -1, 0, "");
            }
            // New method is allowed
            if (cl->screen->getFileTransferPermission!=NULL)
            {
                if (cl->screen->getFileTransferPermission(cl)==TRUE)
                {
                    return rfbSendFileTransferMessage(cl, rfbFileTransferAccess, 0, 1 , 0, ""); // Permit
                }
                else
                {
                    return rfbSendFileTransferMessage(cl, rfbFileTransferAccess, 0, -1 , 0, ""); // Deny
                }
            }
            else
            {
                /**if (cl->permitFileTransfer)
                {
                    return rfbSendFileTransferMessage(cl, rfbFileTransferAccess, 0, 1 , 0, ""); // Permit
                }
                else*/
                {
                    return rfbSendFileTransferMessage(cl, rfbFileTransferAccess, 0, -1 , 0, ""); // DEFAULT: DENY (for security)
                }

            }
        }
        break;


    case rfbCommand:

        //rfbLog("rfbProcessFileTransfer() rfbCommand:\n");

        if ((buffer = rfbProcessFileTransferReadBuffer(cl, length))==NULL) return FALSE;
        switch (contentParam)
        {
        case rfbCDirCreate:  // Client requests the creation of a directory
            rfbFilenameTranslate2UNIX(cl, buffer, filename1);
            retval = CreateDirectoryA(filename1, NULL);
            retval = rfbSendFileTransferMessage(cl, rfbCommandReturn, rfbADirCreate, retval, length, buffer);
            if (buffer!=NULL) MemFree(buffer);
            return retval;
        case rfbCFileDelete: // Client requests the deletion of a file
            rfbFilenameTranslate2UNIX(cl, buffer, filename1);
            dwFileAttributes = GetFileAttributesA(filename1);
            if(dwFileAttributes == INVALID_FILE_ATTRIBUTES)
                retval = -1;
            else if(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                retval = RemoveDirectory(filename1) ? 0 : -1;	// will fail if not empty, but same for _rmdir
            else	// file?
                retval = DeleteFileA(filename1) ? 0 : -1;

            /*if (stat(filename1,&statbuf)==0)
            {
                if (S_ISDIR(statbuf.st_mode))
                    retval = _rmdir(filename1);
                else
                    retval = _unlink(filename1);
            }
            else retval=-1; */
            retval = rfbSendFileTransferMessage(cl, rfbCommandReturn, rfbAFileDelete, retval, length, buffer);
            if (buffer!=NULL) MemFree(buffer);
            return retval;
        case rfbCFileRename: // Client requests the Renaming of a file/directory
            //p = strrchr(buffer, '*');
            p = buffer;
            while(*p && *p != '*') p++;
            if(*p == 0) p = NULL;

            if (p != NULL)
            {
                // Split into 2 filenames ('*' is a seperator)
                *p = '\0';
                rfbFilenameTranslate2UNIX(cl, buffer, filename1);
                rfbFilenameTranslate2UNIX(cl, p+1,    filename2);
                //retval = rename(filename1,filename2);
                retval = MoveFile(filename1, filename2) ? 0 : -1;
                // Restore the buffer so the reply is good
                *p = '*';
                retval = rfbSendFileTransferMessage(cl, rfbCommandReturn, rfbAFileRename, retval, length, buffer);
                if (buffer!=NULL) MemFree(buffer);
                return retval;
            }
            break;
        }

        break;
    }

    // NOTE: don't forget to free(buffer) if you return early!
    if (buffer!=NULL) MemFree(buffer);
    return TRUE;
}

/*
 * rfbProcessClientNormalMessage is called when the client has sent a normal
 * protocol message.
 */

static void
rfbProcessClientNormalMessage(rfbClientPtr cl)
{
    int n=0;
    rfbClientToServerMsg msg;
    char *str;
    int i;
    uint32_t enc=0;
    uint32_t lastPreferredEncoding = -1;

    if ((n = rfbReadExact(cl, (char *)&msg, 1)) <= 0)
    {
        rfbCloseClient(cl);
        return;
    }

    switch (msg.type)
    {

    case rfbSetPixelFormat:

        if ((n = rfbReadExact(cl, ((char *)&msg) + 1,
                              sz_rfbSetPixelFormatMsg - 1)) <= 0)
        {
            rfbCloseClient(cl);
            return;
        }

        cl->format.bitsPerPixel = msg.spf.format.bitsPerPixel;
        cl->format.depth = msg.spf.format.depth;
        cl->format.bigEndian = (msg.spf.format.bigEndian ? TRUE : FALSE);
        cl->format.trueColour = (msg.spf.format.trueColour ? TRUE : FALSE);
        cl->format.redMax = Swap16IfLE(msg.spf.format.redMax);
        cl->format.greenMax = Swap16IfLE(msg.spf.format.greenMax);
        cl->format.blueMax = Swap16IfLE(msg.spf.format.blueMax);
        cl->format.redShift = msg.spf.format.redShift;
        cl->format.greenShift = msg.spf.format.greenShift;
        cl->format.blueShift = msg.spf.format.blueShift;

        cl->readyForSetColourMapEntries = TRUE;
        cl->screen->setTranslateFunction(cl);
        return;

    case rfbFixColourMapEntries:
        if ((n = rfbReadExact(cl, ((char *)&msg) + 1,
                              sz_rfbFixColourMapEntriesMsg - 1)) <= 0)
        {
            rfbCloseClient(cl);
            return;
        }
        rfbCloseClient(cl);
        return;


        /* NOTE: Some clients send us a set of encodings (ie: PointerPos) designed to enable/disable features...
         * We may want to look into this...
         * Example:
         *     case rfbEncodingXCursor:
         *         cl->enableCursorShapeUpdates = TRUE;
         *
         * Currently: cl->enableCursorShapeUpdates can *never* be turned off...
         */
    case rfbSetEncodings:
    {

        if ((n = rfbReadExact(cl, ((char *)&msg) + 1,
                              sz_rfbSetEncodingsMsg - 1)) <= 0)
        {
            rfbCloseClient(cl);
            return;
        }

        msg.se.nEncodings = Swap16IfLE(msg.se.nEncodings);

        /*
         * UltraVNC Client has the ability to adapt to changing network environments
         * So, let's give it a change to tell us what it wants now!
         */
        if (cl->preferredEncoding!=-1)
            lastPreferredEncoding = cl->preferredEncoding;

        /* Reset all flags to defaults (allows us to switch between PointerPos and Server Drawn Cursors) */
        cl->preferredEncoding=-1;
        cl->useCopyRect              = FALSE;
        cl->useNewFBSize             = FALSE;
        cl->cursorWasChanged         = FALSE;
        cl->useRichCursorEncoding    = FALSE;
        cl->enableCursorPosUpdates   = FALSE;
        cl->enableCursorShapeUpdates = FALSE;
        cl->enableLastRectEncoding   = FALSE;
        cl->enableKeyboardLedState   = FALSE;
        cl->enableSupportedMessages  = FALSE;
        cl->enableSupportedEncodings = FALSE;
        cl->enableServerIdentity     = FALSE;


        for (i = 0; i < msg.se.nEncodings; i++)
        {
            if ((n = rfbReadExact(cl, (char *)&enc, 4)) <= 0)
            {
                rfbCloseClient(cl);
                return;
            }
            enc = Swap32IfLE(enc);

            switch (enc)
            {

            case rfbEncodingCopyRect:
                cl->useCopyRect = TRUE;
                break;
            case rfbEncodingRaw:
            case rfbEncodingRRE:
            case rfbEncodingCoRRE:
            case rfbEncodingHextile:
            case rfbEncodingUltra:
#ifdef LIBVNCSERVER_HAVE_LIBZ
            case rfbEncodingZlib:
            case rfbEncodingZRLE:
            case rfbEncodingZYWRLE:
#ifdef LIBVNCSERVER_HAVE_LIBJPEG
            case rfbEncodingTight:
#endif
#endif
                /* The first supported encoding is the 'preferred' encoding */
                if (cl->preferredEncoding == -1)
                    cl->preferredEncoding = enc;


                break;
            case rfbEncodingXCursor:
                if(!cl->screen->dontConvertRichCursorToXCursor)
                {
                    /* if cursor was drawn, hide the cursor */
                    if(!cl->enableCursorShapeUpdates)
                        rfbRedrawAfterHideCursor(cl,NULL);

                    cl->enableCursorShapeUpdates = TRUE;
                    cl->cursorWasChanged = TRUE;
                }
                break;
            case rfbEncodingRichCursor:
                /* if cursor was drawn, hide the cursor */
                if(!cl->enableCursorShapeUpdates)
                    rfbRedrawAfterHideCursor(cl,NULL);

                cl->enableCursorShapeUpdates = TRUE;
                cl->useRichCursorEncoding = TRUE;
                cl->cursorWasChanged = TRUE;
                break;
            case rfbEncodingPointerPos:
                if (!cl->enableCursorPosUpdates)
                {
                    cl->enableCursorPosUpdates = TRUE;
                    cl->cursorWasMoved = TRUE;
                }
                break;
            case rfbEncodingLastRect:
                if (!cl->enableLastRectEncoding)
                {
                    cl->enableLastRectEncoding = TRUE;
                }
                break;
            case rfbEncodingNewFBSize:
                if (!cl->useNewFBSize)
                {
                    cl->useNewFBSize = TRUE;
                }
                break;
            case rfbEncodingKeyboardLedState:
                if (!cl->enableKeyboardLedState)
                {
                    cl->enableKeyboardLedState = TRUE;
                }
                break;
            case rfbEncodingSupportedMessages:
                if (!cl->enableSupportedMessages)
                {
                    cl->enableSupportedMessages = TRUE;
                }
                break;
            case rfbEncodingSupportedEncodings:
                if (!cl->enableSupportedEncodings)
                {
                    cl->enableSupportedEncodings = TRUE;
                }
                break;
            case rfbEncodingServerIdentity:
                if (!cl->enableServerIdentity)
                {
                    cl->enableServerIdentity = TRUE;
                }
                break;
            case rfbEncodingXvp:
                if (!rfbSendXvp(cl, 1, rfbXvp_Init))
                {
                    rfbCloseClient(cl);
                    return;
                }
                break;
            default:
#ifdef LIBVNCSERVER_HAVE_LIBZ
                if ( enc >= (uint32_t)rfbEncodingCompressLevel0 &&
                        enc <= (uint32_t)rfbEncodingCompressLevel9 )
                {
                    cl->zlibCompressLevel = enc & 0x0F;
#ifdef LIBVNCSERVER_HAVE_LIBJPEG
                    cl->tightCompressLevel = enc & 0x0F;
#endif
                }
                else if ( enc >= (uint32_t)rfbEncodingQualityLevel0 &&
                          enc <= (uint32_t)rfbEncodingQualityLevel9 )
                {
                    cl->tightQualityLevel = enc & 0x0F;
                }
                else
#endif
                {
                    rfbExtensionData* e;
                    for(e = cl->extensions; e;)
                    {
                        rfbExtensionData* next = e->next;
                        if(e->extension->enablePseudoEncoding &&
                                e->extension->enablePseudoEncoding(cl,
                                        &e->data, (int)enc))
                            /* ext handles this encoding */
                            break;
                        e = next;
                    }
                    if(e == NULL)
                    {
                        rfbBool handled = FALSE;
                        /* if the pseudo encoding is not handled by the
                           enabled extensions, search through all
                           extensions. */
                        rfbProtocolExtension* e;

                        for(e = rfbGetExtensionIterator(); e;)
                        {
                            int* encs = e->pseudoEncodings;
                            while(encs && *encs!=0)
                            {
                                if(*encs==(int)enc)
                                {
                                    void* data = NULL;
                                    if(e->enablePseudoEncoding(cl, &data, (int)enc))
                                    {
                                        rfbEnableExtension(cl, e, data);
                                        handled = TRUE;
                                        e = NULL;
                                        break;
                                    }
                                }
                                encs++;
                            }

                            if(e)
                                e = e->next;
                        }
                        rfbReleaseExtensionIterator();
                    }
                }
            }
        }



        if (cl->preferredEncoding == -1)
        {
            if (lastPreferredEncoding==-1)
            {
                cl->preferredEncoding = rfbEncodingRaw;
            }
            else
            {
                cl->preferredEncoding = lastPreferredEncoding;
            }
        }

        if (cl->enableCursorPosUpdates && !cl->enableCursorShapeUpdates)
        {
            cl->enableCursorPosUpdates = FALSE;
        }

        return;
    }


    case rfbFramebufferUpdateRequest:
    {
        sraRegionPtr tmpRegion;

        if ((n = rfbReadExact(cl, ((char *)&msg) + 1,
                              sz_rfbFramebufferUpdateRequestMsg-1)) <= 0)
        {
            rfbCloseClient(cl);
            return;
        }

        /* The values come in based on the scaled screen, we need to convert them to
         * values based on the main screen's coordinate system
         */
        if(!rectSwapIfLEAndClip(&msg.fur.x,&msg.fur.y,&msg.fur.w,&msg.fur.h,cl))
        {
            return;
        }


        tmpRegion =
            sraRgnCreateRect(msg.fur.x,
                             msg.fur.y,
                             msg.fur.x+msg.fur.w,
                             msg.fur.y+msg.fur.h);

        LOCK(cl->updateMutex);
        sraRgnOr(cl->requestedRegion,tmpRegion);

        if (!cl->readyForSetColourMapEntries)
        {
            /* client hasn't sent a SetPixelFormat so is using server's */
            cl->readyForSetColourMapEntries = TRUE;
            if (!cl->format.trueColour)
            {
                if (!rfbSetClientColourMap(cl, 0, 0))
                {
                    sraRgnDestroy(tmpRegion);
                    TSIGNAL(cl->updateCond);
                    UNLOCK(cl->updateMutex);
                    return;
                }
            }
        }

        if (!msg.fur.incremental)
        {
            sraRgnOr(cl->modifiedRegion,tmpRegion);
            sraRgnSubtract(cl->copyRegion,tmpRegion);
        }
        TSIGNAL(cl->updateCond);
        UNLOCK(cl->updateMutex);

        sraRgnDestroy(tmpRegion);

        return;
    }

    case rfbKeyEvent:

        if ((n = rfbReadExact(cl, ((char *)&msg) + 1,
                              sz_rfbKeyEventMsg - 1)) <= 0)
        {
            rfbCloseClient(cl);
            return;
        }

        if(!cl->viewOnly)
        {
            cl->screen->kbdAddEvent(msg.ke.down, (rfbKeySym)Swap32IfLE(msg.ke.key), cl);
        }

        return;


    case rfbPointerEvent:

        if ((n = rfbReadExact(cl, ((char *)&msg) + 1,
                              sz_rfbPointerEventMsg - 1)) <= 0)
        {
            rfbCloseClient(cl);
            return;
        }

        if (cl->screen->pointerClient && cl->screen->pointerClient != cl)
            return;

        if (msg.pe.buttonMask == 0)
            cl->screen->pointerClient = NULL;
        else
            cl->screen->pointerClient = cl;

        if(!cl->viewOnly)
        {
            if (msg.pe.buttonMask != cl->lastPtrButtons ||
                    cl->screen->deferPtrUpdateTime == 0)
            {
                cl->screen->ptrAddEvent(msg.pe.buttonMask,
                                        ScaleX(cl->scaledScreen, cl->screen, Swap16IfLE(msg.pe.x)),
                                        ScaleY(cl->scaledScreen, cl->screen, Swap16IfLE(msg.pe.y)),
                                        cl);
                cl->lastPtrButtons = msg.pe.buttonMask;
            }
            else
            {
                cl->lastPtrX = ScaleX(cl->scaledScreen, cl->screen, Swap16IfLE(msg.pe.x));
                cl->lastPtrY = ScaleY(cl->scaledScreen, cl->screen, Swap16IfLE(msg.pe.y));
                cl->lastPtrButtons = msg.pe.buttonMask;
            }
        }
        return;


    case rfbFileTransfer:
        if ((n = rfbReadExact(cl, ((char *)&msg) + 1,
                              sz_rfbFileTransferMsg - 1)) <= 0)
        {
            rfbCloseClient(cl);
            return;
        }
        msg.ft.size         = Swap32IfLE(msg.ft.size);
        msg.ft.length       = Swap32IfLE(msg.ft.length);
        /* record statistics in rfbProcessFileTransfer as length is filled with garbage when it is not valid */
        rfbProcessFileTransfer(cl, msg.ft.contentType, msg.ft.contentParam, msg.ft.size, msg.ft.length);
        return;

    case rfbSetSW:
        if ((n = rfbReadExact(cl, ((char *)&msg) + 1,
                              sz_rfbSetSWMsg - 1)) <= 0)
        {
            rfbCloseClient(cl);
            return;
        }
        msg.sw.x = Swap16IfLE(msg.sw.x);
        msg.sw.y = Swap16IfLE(msg.sw.y);
        /* msg.sw.status is not initialized in the ultraVNC viewer and contains random numbers (why???) */

        if (cl->screen->setSingleWindow!=NULL)
            cl->screen->setSingleWindow(cl, msg.sw.x, msg.sw.y);
        return;

    case rfbSetServerInput:
        if ((n = rfbReadExact(cl, ((char *)&msg) + 1,
                              sz_rfbSetServerInputMsg - 1)) <= 0)
        {
            rfbCloseClient(cl);
            return;
        }

        /* msg.sim.pad is not initialized in the ultraVNC viewer and contains random numbers (why???) */
        /* msg.sim.pad = Swap16IfLE(msg.sim.pad); */

        if (cl->screen->setServerInput!=NULL)
            cl->screen->setServerInput(cl, msg.sim.status);
        return;

    case rfbTextChat:
        if ((n = rfbReadExact(cl, ((char *)&msg) + 1,
                              sz_rfbTextChatMsg - 1)) <= 0)
        {
            rfbCloseClient(cl);
            return;
        }

        msg.tc.pad2   = Swap16IfLE(msg.tc.pad2);
        msg.tc.length = Swap32IfLE(msg.tc.length);

        switch (msg.tc.length)
        {
        case rfbTextChatOpen:
        case rfbTextChatClose:
        case rfbTextChatFinished:
            /* commands do not have text following */
            /* Why couldn't they have used the pad byte??? */
            str=NULL;
            break;
        default:
            if ((msg.tc.length>0) && (msg.tc.length<rfbTextMaxSize))
            {
                str = (char *)MemAlloc(msg.tc.length);
                if (str==NULL)
                {
                    rfbCloseClient(cl);
                    return;
                }
                if ((n = rfbReadExact(cl, str, msg.tc.length)) <= 0)
                {
                    MemFree(str);
                    rfbCloseClient(cl);
                    return;
                }
            }
            else
            {
                rfbCloseClient(cl);
                return;
            }
        }

        /* Note: length can be commands: rfbTextChatOpen, rfbTextChatClose, and rfbTextChatFinished
         * at which point, the str is NULL (as it is not sent)
         */
        if (cl->screen->setTextChat!=NULL)
            cl->screen->setTextChat(cl, msg.tc.length, str);

        MemFree(str);
        return;


    case rfbClientCutText:

        if ((n = rfbReadExact(cl, ((char *)&msg) + 1,
                              sz_rfbClientCutTextMsg - 1)) <= 0)
        {
            rfbCloseClient(cl);
            return;
        }

        msg.cct.length = Swap32IfLE(msg.cct.length);

        str = (char *)MemAlloc(msg.cct.length);

        if ((n = rfbReadExact(cl, str, msg.cct.length)) <= 0)
        {
            MemFree(str);
            rfbCloseClient(cl);
            return;
        }
        if((!cl->viewOnly) && (cl->allowClipboard))
        {
            cl->screen->setXCutText(str, msg.cct.length, cl);
        }
        MemFree(str);

        return;

    case rfbPalmVNCSetScaleFactor:
        cl->PalmVNC = TRUE;
        if ((n = rfbReadExact(cl, ((char *)&msg) + 1,
                              sz_rfbSetScaleMsg - 1)) <= 0)
        {
            rfbCloseClient(cl);
            return;
        }
        rfbScalingSetup(cl,cl->screen->width/msg.ssc.scale, cl->screen->height/msg.ssc.scale);

        rfbSendNewScaleSize(cl);
        return;

    case rfbSetScale:

        if ((n = rfbReadExact(cl, ((char *)&msg) + 1,
                              sz_rfbSetScaleMsg - 1)) <= 0)
        {
            rfbCloseClient(cl);
            return;
        }
        rfbScalingSetup(cl,cl->screen->width/msg.ssc.scale, cl->screen->height/msg.ssc.scale);

        rfbSendNewScaleSize(cl);
        return;

    case rfbXvp:

        if ((n = rfbReadExact(cl, ((char *)&msg) + 1,
                              sz_rfbXvpMsg - 1)) <= 0)
        {
            rfbCloseClient(cl);
            return;
        }
        /* only version when is defined, so echo back a fail */
        if(msg.xvp.version != 1)
        {
            rfbSendXvp(cl, msg.xvp.version, rfbXvp_Fail);
        }
        else
        {
            /* if the hook exists and fails, send a fail msg */
            if(cl->screen->xvpHook && !cl->screen->xvpHook(cl, msg.xvp.version, msg.xvp.code))
                rfbSendXvp(cl, 1, rfbXvp_Fail);
        }
        return;

    default:
    {
        rfbExtensionData *e,*next;

        for(e=cl->extensions; e;)
        {
            next = e->next;
            if(e->extension->handleMessage &&
                    e->extension->handleMessage(cl, e->data, &msg))
            {
                return;
            }
            e = next;
        }

        rfbCloseClient(cl);
        return;
    }
    }
}



/*
 * rfbSendFramebufferUpdate - send the currently pending framebuffer update to
 * the RFB client.
 * givenUpdateRegion is not changed.
 */

rfbBool
rfbSendFramebufferUpdate(rfbClientPtr cl,
                         sraRegionPtr givenUpdateRegion)
{
    sraRectangleIterator* i=NULL;
    sraRect rect;
    int nUpdateRegionRects;
    rfbFramebufferUpdateMsg *fu = (rfbFramebufferUpdateMsg *)cl->updateBuf;
    sraRegionPtr updateRegion,updateCopyRegion,tmpRegion;
    int dx, dy;
    rfbBool sendCursorShape = FALSE;
    rfbBool sendCursorPos = FALSE;
    rfbBool sendKeyboardLedState = FALSE;
    rfbBool sendSupportedMessages = FALSE;
    rfbBool sendSupportedEncodings = FALSE;
    rfbBool sendServerIdentity = FALSE;
    rfbBool result = TRUE;


    if(cl->screen->displayHook)
        cl->screen->displayHook(cl);

    /*
     * If framebuffer size was changed and the client supports NewFBSize
     * encoding, just send NewFBSize marker and return.
     */

    if (cl->useNewFBSize && cl->newFBSizePending)
    {
        LOCK(cl->updateMutex);
        cl->newFBSizePending = FALSE;
        UNLOCK(cl->updateMutex);
        fu->type = rfbFramebufferUpdate;
        fu->nRects = Swap16IfLE(1);
        cl->ublen = sz_rfbFramebufferUpdateMsg;
        if (!rfbSendNewFBSize(cl, cl->scaledScreen->width, cl->scaledScreen->height))
        {
            if(cl->screen->displayFinishedHook)
                cl->screen->displayFinishedHook(cl, FALSE);
            return FALSE;
        }
        result = rfbSendUpdateBuf(cl);
        if(cl->screen->displayFinishedHook)
            cl->screen->displayFinishedHook(cl, result);
        return result;
    }

    /*
     * If this client understands cursor shape updates, cursor should be
     * removed from the framebuffer. Otherwise, make sure it's put up.
     */

    if (cl->enableCursorShapeUpdates)
    {
        if (cl->cursorWasChanged && cl->readyForSetColourMapEntries)
            sendCursorShape = TRUE;
    }

    /*
     * Do we plan to send cursor position update?
     */

    if (cl->enableCursorPosUpdates && cl->cursorWasMoved)
        sendCursorPos = TRUE;

    /*
     * Do we plan to send a keyboard state update?
     */
    if ((cl->enableKeyboardLedState) &&
            (cl->screen->getKeyboardLedStateHook!=NULL))
    {
        int x;
        x=cl->screen->getKeyboardLedStateHook(cl->screen);
        if (x!=cl->lastKeyboardLedState)
        {
            sendKeyboardLedState = TRUE;
            cl->lastKeyboardLedState=x;
        }
    }

    /*
     * Do we plan to send a rfbEncodingSupportedMessages?
     */
    if (cl->enableSupportedMessages)
    {
        sendSupportedMessages = TRUE;
        /* We only send this message ONCE <per setEncodings message received>
         * (We disable it here)
         */
        cl->enableSupportedMessages = FALSE;
    }
    /*
     * Do we plan to send a rfbEncodingSupportedEncodings?
     */
    if (cl->enableSupportedEncodings)
    {
        sendSupportedEncodings = TRUE;
        /* We only send this message ONCE <per setEncodings message received>
         * (We disable it here)
         */
        cl->enableSupportedEncodings = FALSE;
    }
    /*
     * Do we plan to send a rfbEncodingServerIdentity?
     */
    if (cl->enableServerIdentity)
    {
        sendServerIdentity = TRUE;
        /* We only send this message ONCE <per setEncodings message received>
         * (We disable it here)
         */
        cl->enableServerIdentity = FALSE;
    }

    LOCK(cl->updateMutex);

    /*
     * The modifiedRegion may overlap the destination copyRegion.  We remove
     * any overlapping bits from the copyRegion (since they'd only be
     * overwritten anyway).
     */

    sraRgnSubtract(cl->copyRegion,cl->modifiedRegion);

    /*
     * The client is interested in the region requestedRegion.  The region
     * which should be updated now is the intersection of requestedRegion
     * and the union of modifiedRegion and copyRegion.  If it's empty then
     * no update is needed.
     */

    updateRegion = sraRgnCreateRgn(givenUpdateRegion);
    if(cl->screen->progressiveSliceHeight>0)
    {
        int height=cl->screen->progressiveSliceHeight,
            y=cl->progressiveSliceY;
        sraRegionPtr bbox=sraRgnBBox(updateRegion);
        sraRect rect;
        if(sraRgnPopRect(bbox,&rect,0))
        {
            sraRegionPtr slice;
            if(y<rect.y1 || y>=rect.y2)
                y=rect.y1;
            slice=sraRgnCreateRect(0,y,cl->screen->width,y+height);
            sraRgnAnd(updateRegion,slice);
            sraRgnDestroy(slice);
        }
        sraRgnDestroy(bbox);
        y+=height;
        if(y>=cl->screen->height)
            y=0;
        cl->progressiveSliceY=y;
    }

    sraRgnOr(updateRegion,cl->copyRegion);
    if(!sraRgnAnd(updateRegion,cl->requestedRegion) &&
            sraRgnEmpty(updateRegion) &&
            (cl->enableCursorShapeUpdates ||
             (cl->cursorX == cl->screen->cursorX && cl->cursorY == cl->screen->cursorY)) &&
            !sendCursorShape && !sendCursorPos && !sendKeyboardLedState &&
            !sendSupportedMessages && !sendSupportedEncodings && !sendServerIdentity)
    {
        sraRgnDestroy(updateRegion);
        UNLOCK(cl->updateMutex);
        if(cl->screen->displayFinishedHook)
            cl->screen->displayFinishedHook(cl, TRUE);
        return TRUE;
    }

    /*
     * We assume that the client doesn't have any pixel data outside the
     * requestedRegion.  In other words, both the source and destination of a
     * copy must lie within requestedRegion.  So the region we can send as a
     * copy is the intersection of the copyRegion with both the requestedRegion
     * and the requestedRegion translated by the amount of the copy.  We set
     * updateCopyRegion to this.
     */

    updateCopyRegion = sraRgnCreateRgn(cl->copyRegion);
    sraRgnAnd(updateCopyRegion,cl->requestedRegion);
    tmpRegion = sraRgnCreateRgn(cl->requestedRegion);
    sraRgnOffset(tmpRegion,cl->copyDX,cl->copyDY);
    sraRgnAnd(updateCopyRegion,tmpRegion);
    sraRgnDestroy(tmpRegion);
    dx = cl->copyDX;
    dy = cl->copyDY;

    /*
     * Next we remove updateCopyRegion from updateRegion so that updateRegion
     * is the part of this update which is sent as ordinary pixel data (i.e not
     * a copy).
     */

    sraRgnSubtract(updateRegion,updateCopyRegion);

    /*
     * Finally we leave modifiedRegion to be the remainder (if any) of parts of
     * the screen which are modified but outside the requestedRegion.  We also
     * empty both the requestedRegion and the copyRegion - note that we never
     * carry over a copyRegion for a future update.
     */

    sraRgnOr(cl->modifiedRegion,cl->copyRegion);
    sraRgnSubtract(cl->modifiedRegion,updateRegion);
    sraRgnSubtract(cl->modifiedRegion,updateCopyRegion);

    sraRgnMakeEmpty(cl->requestedRegion);
    sraRgnMakeEmpty(cl->copyRegion);
    cl->copyDX = 0;
    cl->copyDY = 0;

    UNLOCK(cl->updateMutex);

    if (!cl->enableCursorShapeUpdates)
    {
        if(cl->cursorX != cl->screen->cursorX || cl->cursorY != cl->screen->cursorY)
        {
            rfbRedrawAfterHideCursor(cl,updateRegion);
            LOCK(cl->screen->cursorMutex);
            cl->cursorX = cl->screen->cursorX;
            cl->cursorY = cl->screen->cursorY;
            UNLOCK(cl->screen->cursorMutex);
            rfbRedrawAfterHideCursor(cl,updateRegion);
        }
        rfbShowCursor(cl);
    }

    /*
     * Now send the update.
     */

    if (cl->preferredEncoding == rfbEncodingCoRRE)
    {
        nUpdateRegionRects = 0;

        for(i = sraRgnGetIterator(updateRegion); sraRgnIteratorNext(i,&rect);)
        {
            int x = rect.x1;
            int y = rect.y1;
            int w = rect.x2 - x;
            int h = rect.y2 - y;
            int rectsPerRow, rows;
            /* We need to count the number of rects in the scaled screen */
            if (cl->screen!=cl->scaledScreen)
                rfbScaledCorrection(cl->screen, cl->scaledScreen, &x, &y, &w, &h);
            rectsPerRow = (w-1)/cl->correMaxWidth+1;
            rows = (h-1)/cl->correMaxHeight+1;
            nUpdateRegionRects += rectsPerRow*rows;
        }
        sraRgnReleaseIterator(i);
        i=NULL;
    }
    else if (cl->preferredEncoding == rfbEncodingUltra)
    {
        nUpdateRegionRects = 0;

        for(i = sraRgnGetIterator(updateRegion); sraRgnIteratorNext(i,&rect);)
        {
            int x = rect.x1;
            int y = rect.y1;
            int w = rect.x2 - x;
            int h = rect.y2 - y;
            /* We need to count the number of rects in the scaled screen */
            if (cl->screen!=cl->scaledScreen)
                rfbScaledCorrection(cl->screen, cl->scaledScreen, &x, &y, &w, &h);
            nUpdateRegionRects += (((h-1) / (ULTRA_MAX_SIZE( w ) / w)) + 1);
        }
        sraRgnReleaseIterator(i);
        i=NULL;
#ifdef LIBVNCSERVER_HAVE_LIBZ
    }
    else if (cl->preferredEncoding == rfbEncodingZlib)
    {
        nUpdateRegionRects = 0;

        for(i = sraRgnGetIterator(updateRegion); sraRgnIteratorNext(i,&rect);)
        {
            int x = rect.x1;
            int y = rect.y1;
            int w = rect.x2 - x;
            int h = rect.y2 - y;
            /* We need to count the number of rects in the scaled screen */
            if (cl->screen!=cl->scaledScreen)
                rfbScaledCorrection(cl->screen, cl->scaledScreen, &x, &y, &w, &h);
            nUpdateRegionRects += (((h-1) / (ZLIB_MAX_SIZE( w ) / w)) + 1);
        }
        sraRgnReleaseIterator(i);
        i=NULL;
#ifdef LIBVNCSERVER_HAVE_LIBJPEG
    }
    else if (cl->preferredEncoding == rfbEncodingTight)
    {
        nUpdateRegionRects = 0;

        for(i = sraRgnGetIterator(updateRegion); sraRgnIteratorNext(i,&rect);)
        {
            int x = rect.x1;
            int y = rect.y1;
            int w = rect.x2 - x;
            int h = rect.y2 - y;
            int n;
            /* We need to count the number of rects in the scaled screen */
            if (cl->screen!=cl->scaledScreen)
                rfbScaledCorrection(cl->screen, cl->scaledScreen, &x, &y, &w, &h);
            n = rfbNumCodedRectsTight(cl, x, y, w, h);
            if (n == 0)
            {
                nUpdateRegionRects = 0xFFFF;
                break;
            }
            nUpdateRegionRects += n;
        }
        sraRgnReleaseIterator(i);
        i=NULL;
#endif
#endif
    }
    else
    {
        nUpdateRegionRects = sraRgnCountRects(updateRegion);
    }

    fu->type = rfbFramebufferUpdate;
    if (nUpdateRegionRects != 0xFFFF)
    {
        if(cl->screen->maxRectsPerUpdate>0
                /* CoRRE splits the screen into smaller squares */
                && cl->preferredEncoding != rfbEncodingCoRRE
                /* Ultra encoding splits rectangles up into smaller chunks */
                && cl->preferredEncoding != rfbEncodingUltra
#ifdef LIBVNCSERVER_HAVE_LIBZ
                /* Zlib encoding splits rectangles up into smaller chunks */
                && cl->preferredEncoding != rfbEncodingZlib
#ifdef LIBVNCSERVER_HAVE_LIBJPEG
                /* Tight encoding counts the rectangles differently */
                && cl->preferredEncoding != rfbEncodingTight
#endif
#endif
                && nUpdateRegionRects>cl->screen->maxRectsPerUpdate)
        {
            sraRegion* newUpdateRegion = sraRgnBBox(updateRegion);
            sraRgnDestroy(updateRegion);
            updateRegion = newUpdateRegion;
            nUpdateRegionRects = sraRgnCountRects(updateRegion);
        }
        fu->nRects = Swap16IfLE((uint16_t)(sraRgnCountRects(updateCopyRegion) +
                                           nUpdateRegionRects +
                                           !!sendCursorShape + !!sendCursorPos + !!sendKeyboardLedState +
                                           !!sendSupportedMessages + !!sendSupportedEncodings + !!sendServerIdentity));
    }
    else
    {
        fu->nRects = 0xFFFF;
    }
    cl->ublen = sz_rfbFramebufferUpdateMsg;

    if (sendCursorShape)
    {
        cl->cursorWasChanged = FALSE;
        if (!rfbSendCursorShape(cl))
            goto updateFailed;
    }

    if (sendCursorPos)
    {
        cl->cursorWasMoved = FALSE;
        if (!rfbSendCursorPos(cl))
            goto updateFailed;
    }

    if (sendKeyboardLedState)
    {
        if (!rfbSendKeyboardLedState(cl))
            goto updateFailed;
    }

    if (sendSupportedMessages)
    {
        if (!rfbSendSupportedMessages(cl))
            goto updateFailed;
    }
    if (sendSupportedEncodings)
    {
        if (!rfbSendSupportedEncodings(cl))
            goto updateFailed;
    }
    if (sendServerIdentity)
    {
        if (!rfbSendServerIdentity(cl))
            goto updateFailed;
    }

    if (!sraRgnEmpty(updateCopyRegion))
    {
        if (!rfbSendCopyRegion(cl,updateCopyRegion,dx,dy))
            goto updateFailed;
    }

    for(i = sraRgnGetIterator(updateRegion); sraRgnIteratorNext(i,&rect);)
    {
        int x = rect.x1;
        int y = rect.y1;
        int w = rect.x2 - x;
        int h = rect.y2 - y;

        /* We need to count the number of rects in the scaled screen */
        if (cl->screen!=cl->scaledScreen)
            rfbScaledCorrection(cl->screen, cl->scaledScreen, &x, &y, &w, &h);

        switch (cl->preferredEncoding)
        {
        case -1:
        case rfbEncodingRaw:
            if (!rfbSendRectEncodingRaw(cl, x, y, w, h))
                goto updateFailed;
            break;
        case rfbEncodingRRE:
            if (!rfbSendRectEncodingRRE(cl, x, y, w, h))
                goto updateFailed;
            break;
        case rfbEncodingCoRRE:
            if (!rfbSendRectEncodingCoRRE(cl, x, y, w, h))
                goto updateFailed;
            break;
        case rfbEncodingHextile:
            if (!rfbSendRectEncodingHextile(cl, x, y, w, h))
                goto updateFailed;
            break;
        case rfbEncodingUltra:
            if (!rfbSendRectEncodingUltra(cl, x, y, w, h))
                goto updateFailed;
            break;
#ifdef LIBVNCSERVER_HAVE_LIBZ
        case rfbEncodingZlib:
            if (!rfbSendRectEncodingZlib(cl, x, y, w, h))
                goto updateFailed;
            break;
#ifdef LIBVNCSERVER_HAVE_LIBJPEG
        case rfbEncodingTight:
            if (!rfbSendRectEncodingTight(cl, x, y, w, h))
                goto updateFailed;
            break;
#endif
#endif
#ifdef LIBVNCSERVER_HAVE_LIBZ
        case rfbEncodingZRLE:
        case rfbEncodingZYWRLE:
            if (!rfbSendRectEncodingZRLE(cl, x, y, w, h))
                goto updateFailed;
            break;
#endif
        }
    }
    if (i)
    {
        sraRgnReleaseIterator(i);
        i = NULL;
    }

    if ( nUpdateRegionRects == 0xFFFF &&
            !rfbSendLastRectMarker(cl) )
        goto updateFailed;

    if (!rfbSendUpdateBuf(cl))
    {
updateFailed:
        result = FALSE;
    }

    if (!cl->enableCursorShapeUpdates)
    {
        rfbHideCursor(cl);
    }

    if(i)
        sraRgnReleaseIterator(i);
    sraRgnDestroy(updateRegion);
    sraRgnDestroy(updateCopyRegion);

    if(cl->screen->displayFinishedHook)
        cl->screen->displayFinishedHook(cl, result);
    return result;
}


/*
 * Send the copy region as a string of CopyRect encoded rectangles.
 * The only slightly tricky thing is that we should send the messages in
 * the correct order so that an earlier CopyRect will not corrupt the source
 * of a later one.
 */

rfbBool
rfbSendCopyRegion(rfbClientPtr cl,
                  sraRegionPtr reg,
                  int dx,
                  int dy)
{
    int x, y, w, h;
    rfbFramebufferUpdateRectHeader rect;
    rfbCopyRect cr;
    sraRectangleIterator* i;
    sraRect rect1;

    /* printf("copyrect: "); sraRgnPrint(reg); putchar('\n');fflush(stdout); */
    i = sraRgnGetReverseIterator(reg,dx>0,dy>0);

    /* correct for the scale of the screen */
    dx = ScaleX(cl->screen, cl->scaledScreen, dx);
    dy = ScaleX(cl->screen, cl->scaledScreen, dy);

    while(sraRgnIteratorNext(i,&rect1))
    {
        x = rect1.x1;
        y = rect1.y1;
        w = rect1.x2 - x;
        h = rect1.y2 - y;

        /* correct for scaling (if necessary) */
        rfbScaledCorrection(cl->screen, cl->scaledScreen, &x, &y, &w, &h);

        rect.r.x = Swap16IfLE(x);
        rect.r.y = Swap16IfLE(y);
        rect.r.w = Swap16IfLE(w);
        rect.r.h = Swap16IfLE(h);
        rect.encoding = Swap32IfLE(rfbEncodingCopyRect);

        memcpy(&cl->updateBuf[cl->ublen], (char *)&rect,
               sz_rfbFramebufferUpdateRectHeader);
        cl->ublen += sz_rfbFramebufferUpdateRectHeader;

        cr.srcX = Swap16IfLE(x - dx);
        cr.srcY = Swap16IfLE(y - dy);

        memcpy(&cl->updateBuf[cl->ublen], (char *)&cr, sz_rfbCopyRect);
        cl->ublen += sz_rfbCopyRect;

    }
    sraRgnReleaseIterator(i);

    return TRUE;
}

/*
 * Send a given rectangle in raw encoding (rfbEncodingRaw).
 */

rfbBool
rfbSendRectEncodingRaw(rfbClientPtr cl,
                       int x,
                       int y,
                       int w,
                       int h)
{
    rfbFramebufferUpdateRectHeader rect;
    int nlines;
    int bytesPerLine = w * (cl->format.bitsPerPixel / 8);
    char *fbptr = (cl->scaledScreen->frameBuffer + (cl->scaledScreen->paddedWidthInBytes * y)
                   + (x * (cl->scaledScreen->bitsPerPixel / 8)));

    /* Flush the buffer to guarantee correct alignment for translateFn(). */
    if (cl->ublen > 0)
    {
        if (!rfbSendUpdateBuf(cl))
            return FALSE;
    }

    rect.r.x = Swap16IfLE(x);
    rect.r.y = Swap16IfLE(y);
    rect.r.w = Swap16IfLE(w);
    rect.r.h = Swap16IfLE(h);
    rect.encoding = Swap32IfLE(rfbEncodingRaw);

    memcpy(&cl->updateBuf[cl->ublen], (char *)&rect,sz_rfbFramebufferUpdateRectHeader);
    cl->ublen += sz_rfbFramebufferUpdateRectHeader;

    nlines = (UPDATE_BUF_SIZE - cl->ublen) / bytesPerLine;

    while (TRUE)
    {
        if (nlines > h)
            nlines = h;

        (*cl->translateFn)(cl->translateLookupTable,
                           &(cl->screen->serverFormat),
                           &cl->format, fbptr, &cl->updateBuf[cl->ublen],
                           cl->scaledScreen->paddedWidthInBytes, w, nlines);

        cl->ublen += nlines * bytesPerLine;
        h -= nlines;

        if (h == 0)     /* rect fitted in buffer, do next one */
            return TRUE;

        /* buffer full - flush partial rect and do another nlines */

        if (!rfbSendUpdateBuf(cl))
            return FALSE;

        fbptr += (cl->scaledScreen->paddedWidthInBytes * nlines);

        nlines = (UPDATE_BUF_SIZE - cl->ublen) / bytesPerLine;
        if (nlines == 0)
        {
            rfbCloseClient(cl);
            return FALSE;
        }
    }
}



/*
 * Send an empty rectangle with encoding field set to value of
 * rfbEncodingLastRect to notify client that this is the last
 * rectangle in framebuffer update ("LastRect" extension of RFB
 * protocol).
 */

rfbBool
rfbSendLastRectMarker(rfbClientPtr cl)
{
    rfbFramebufferUpdateRectHeader rect;

    if (cl->ublen + sz_rfbFramebufferUpdateRectHeader > UPDATE_BUF_SIZE)
    {
        if (!rfbSendUpdateBuf(cl))
            return FALSE;
    }

    rect.encoding = Swap32IfLE(rfbEncodingLastRect);
    rect.r.x = 0;
    rect.r.y = 0;
    rect.r.w = 0;
    rect.r.h = 0;

    memcpy(&cl->updateBuf[cl->ublen], (char *)&rect,sz_rfbFramebufferUpdateRectHeader);
    cl->ublen += sz_rfbFramebufferUpdateRectHeader;

    return TRUE;
}


/*
 * Send NewFBSize pseudo-rectangle. This tells the client to change
 * its framebuffer size.
 */

rfbBool
rfbSendNewFBSize(rfbClientPtr cl,
                 int w,
                 int h)
{
    rfbFramebufferUpdateRectHeader rect;

    if (cl->ublen + sz_rfbFramebufferUpdateRectHeader > UPDATE_BUF_SIZE)
    {
        if (!rfbSendUpdateBuf(cl))
            return FALSE;
    }

    rect.encoding = Swap32IfLE(rfbEncodingNewFBSize);
    rect.r.x = 0;
    rect.r.y = 0;
    rect.r.w = Swap16IfLE(w);
    rect.r.h = Swap16IfLE(h);

    memcpy(&cl->updateBuf[cl->ublen], (char *)&rect,
           sz_rfbFramebufferUpdateRectHeader);
    cl->ublen += sz_rfbFramebufferUpdateRectHeader;

    return TRUE;
}


/*
 * Send the contents of cl->updateBuf.  Returns 1 if successful, -1 if
 * not (errno should be set).
 */

rfbBool
rfbSendUpdateBuf(rfbClientPtr cl)
{
    if(cl->sock<0)
        return FALSE;

    if (rfbWriteExact(cl, cl->updateBuf, cl->ublen) < 0)
    {
        rfbCloseClient(cl);
        return FALSE;
    }

    cl->ublen = 0;
    return TRUE;
}

/*
 * rfbSendSetColourMapEntries sends a SetColourMapEntries message to the
 * client, using values from the currently installed colormap.
 */

rfbBool
rfbSendSetColourMapEntries(rfbClientPtr cl,
                           int firstColour,
                           int nColours)
{
    char buf[sz_rfbSetColourMapEntriesMsg + 256 * 3 * 2];
    char *wbuf = buf;
    rfbSetColourMapEntriesMsg *scme;
    uint16_t *rgb;
    rfbColourMap* cm = &cl->screen->colourMap;
    int i, len;

    if (nColours > 256)
    {
        /* some rare hardware has, e.g., 4096 colors cells: PseudoColor:12 */
        wbuf = (char *) MemAlloc(sz_rfbSetColourMapEntriesMsg + nColours * 3 * 2);
    }

    scme = (rfbSetColourMapEntriesMsg *)wbuf;
    rgb = (uint16_t *)(&wbuf[sz_rfbSetColourMapEntriesMsg]);

    scme->type = rfbSetColourMapEntries;

    scme->firstColour = Swap16IfLE(firstColour);
    scme->nColours = Swap16IfLE(nColours);

    len = sz_rfbSetColourMapEntriesMsg;

    for (i = 0; i < nColours; i++)
    {
        if(i<(int)cm->count)
        {
            if(cm->is16)
            {
                rgb[i*3] = Swap16IfLE(cm->data.shorts[i*3]);
                rgb[i*3+1] = Swap16IfLE(cm->data.shorts[i*3+1]);
                rgb[i*3+2] = Swap16IfLE(cm->data.shorts[i*3+2]);
            }
            else
            {
                rgb[i*3] = Swap16IfLE((unsigned short)cm->data.bytes[i*3]);
                rgb[i*3+1] = Swap16IfLE((unsigned short)cm->data.bytes[i*3+1]);
                rgb[i*3+2] = Swap16IfLE((unsigned short)cm->data.bytes[i*3+2]);
            }
        }
    }

    len += nColours * 3 * 2;

    LOCK(cl->sendMutex);
    if (rfbWriteExact(cl, wbuf, len) < 0)
    {
        rfbCloseClient(cl);
        if (wbuf != buf) MemFree(wbuf);
        UNLOCK(cl->sendMutex);
        return FALSE;
    }
    UNLOCK(cl->sendMutex);

    if (wbuf != buf) MemFree(wbuf);
    return TRUE;
}

/*
 * rfbSendBell sends a Bell message to all the clients.
 */

void
rfbSendBell(rfbScreenInfoPtr rfbScreen)
{
    rfbClientIteratorPtr i;
    rfbClientPtr cl;
    rfbBellMsg b;

    i = rfbGetClientIterator(rfbScreen);
    while((cl=rfbClientIteratorNext(i)))
    {
        b.type = rfbBell;
        LOCK(cl->sendMutex);
        if (rfbWriteExact(cl, (char *)&b, sz_rfbBellMsg) < 0)
        {
            rfbCloseClient(cl);
        }
        UNLOCK(cl->sendMutex);
    }
    rfbReleaseClientIterator(i);
}


/*
 * rfbSendServerCutText sends a ServerCutText message to all the clients.
 */

void
rfbSendServerCutText(rfbScreenInfoPtr rfbScreen,char *str, int len)
{
    rfbClientPtr cl;
    rfbServerCutTextMsg sct;
    rfbClientIteratorPtr iterator;

    iterator = rfbGetClientIterator(rfbScreen);
    while ((cl = rfbClientIteratorNext(iterator)) != NULL)
    {
        sct.type = rfbServerCutText;
        sct.length = Swap32IfLE(len);
        LOCK(cl->sendMutex);
        if (rfbWriteExact(cl, (char *)&sct,
                          sz_rfbServerCutTextMsg) < 0)
        {
            rfbCloseClient(cl);
            UNLOCK(cl->sendMutex);
            continue;
        }
        if (rfbWriteExact(cl, str, len) < 0)
        {
            rfbCloseClient(cl);
        }
        UNLOCK(cl->sendMutex);
    }
    rfbReleaseClientIterator(iterator);
}

/*****************************************************************************
 *
 * UDP can be used for keyboard and pointer events when the underlying
 * network is highly reliable.  This is really here to support ORL's
 * videotile, whose TCP implementation doesn't like sending lots of small
 * packets (such as 100s of pen readings per second!).
 */

static unsigned char ptrAcceleration = 50;

void
rfbNewUDPConnection(rfbScreenInfoPtr rfbScreen,
                    int sock)
{
    x_write(sock, &ptrAcceleration, 1);
}

/*
 * Because UDP is a message based service, we can't read the first byte and
 * then the rest of the packet separately like we do with TCP.  We will always
 * get a whole packet delivered in one go, so we ask read() for the maximum
 * number of bytes we can possibly get.
 */

void
rfbProcessUDPInput(rfbScreenInfoPtr rfbScreen)
{
    int n;
    rfbClientPtr cl=rfbScreen->udpClient;
    rfbClientToServerMsg msg;

    if((!cl) || cl->onHold)
        return;
    //if ((n = _read(rfbScreen->udpSock, (char *)&msg, sizeof(msg))) <= 0) {
    if ((n = recv(rfbScreen->udpSock, (char *)&msg, sizeof(msg), 0)) <= 0)
    {
        rfbDisconnectUDPSock(rfbScreen);
        return;
    }

    switch (msg.type)
    {

    case rfbKeyEvent:
        if (n != sz_rfbKeyEventMsg)
        {
            rfbDisconnectUDPSock(rfbScreen);
            return;
        }
        cl->screen->kbdAddEvent(msg.ke.down, (rfbKeySym)Swap32IfLE(msg.ke.key), cl);
        break;

    case rfbPointerEvent:
        if (n != sz_rfbPointerEventMsg)
        {
            rfbDisconnectUDPSock(rfbScreen);
            return;
        }
        cl->screen->ptrAddEvent(msg.pe.buttonMask,
                                Swap16IfLE(msg.pe.x), Swap16IfLE(msg.pe.y), cl);
        break;

    default:
        rfbDisconnectUDPSock(rfbScreen);
    }
}


