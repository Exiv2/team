#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#define  FSEEK_LONG  long
#pragma  warning(disable : 4996)
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#define  fileno      _fileno
#define  vsnprint    _vsnprintf
#endif

#if defined(__MINGW64__) || defined(__CYGWIN__)
#include <windows.h>
#endif

#include <iostream>
#include <set>
#include <vector>
#include <memory>
#include <cstring>
#include <string>
#include <sstream>
#include <map>
#include <cstdlib>
#include <cassert>
#include <algorithm>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdarg.h>

#ifndef  _MSC_VER
#include <unistd.h>
#define  FSEEK_LONG uint64_t
#else
#include <stdio.h>
#define  STDIN_FILENO _fileno(stdin)
#endif

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

typedef std::set<uint64_t> Visits;
typedef unsigned char Byte;

// https://github.com/Exiv2/exiv2/pull/906#issuecomment-504338797
static void sonyCipher(Byte* bytes, uint32_t size, bool bDecipher)
{
    if ( size && bytes ) {
		// initialize the code table
		Byte  code[256];
		for ( uint32_t i = 0 ; i < 249 ; i++ ) {
			if ( bDecipher ) {
				code[(i * i * i) % 249] = i ;
			} else {
				code[i] = (i * i * i) % 249 ;
			}
		}
		for ( uint32_t i = 249 ; i < 256 ; i++ ) {
			code[i] = i;
		}

        Byte* buff = (Byte*) ::malloc(size);
		// code byte-by-byte
		for ( uint32_t i = 0 ; i < size ; i++ ) {
			buff[i] = code[bytes[i]];
		}

		// copy buff back into bytes
		memcpy(bytes,buff,size);
        ::free(buff);
	}
}

// https://github.com/ncruces/dcraw/blob/master/parse.c
typedef unsigned char uchar;
void nikon_decrypt (uchar ci, uchar cj, uint16_t tag, uint64_t start, uint64_t size, uchar *buf)
{
  static const uchar xlat[2][256] = {
  { 0xc1,0xbf,0x6d,0x0d,0x59,0xc5,0x13,0x9d,0x83,0x61,0x6b,0x4f,0xc7,0x7f,0x3d,0x3d,
    0x53,0x59,0xe3,0xc7,0xe9,0x2f,0x95,0xa7,0x95,0x1f,0xdf,0x7f,0x2b,0x29,0xc7,0x0d,
    0xdf,0x07,0xef,0x71,0x89,0x3d,0x13,0x3d,0x3b,0x13,0xfb,0x0d,0x89,0xc1,0x65,0x1f,
    0xb3,0x0d,0x6b,0x29,0xe3,0xfb,0xef,0xa3,0x6b,0x47,0x7f,0x95,0x35,0xa7,0x47,0x4f,
    0xc7,0xf1,0x59,0x95,0x35,0x11,0x29,0x61,0xf1,0x3d,0xb3,0x2b,0x0d,0x43,0x89,0xc1,
    0x9d,0x9d,0x89,0x65,0xf1,0xe9,0xdf,0xbf,0x3d,0x7f,0x53,0x97,0xe5,0xe9,0x95,0x17,
    0x1d,0x3d,0x8b,0xfb,0xc7,0xe3,0x67,0xa7,0x07,0xf1,0x71,0xa7,0x53,0xb5,0x29,0x89,
    0xe5,0x2b,0xa7,0x17,0x29,0xe9,0x4f,0xc5,0x65,0x6d,0x6b,0xef,0x0d,0x89,0x49,0x2f,
    0xb3,0x43,0x53,0x65,0x1d,0x49,0xa3,0x13,0x89,0x59,0xef,0x6b,0xef,0x65,0x1d,0x0b,
    0x59,0x13,0xe3,0x4f,0x9d,0xb3,0x29,0x43,0x2b,0x07,0x1d,0x95,0x59,0x59,0x47,0xfb,
    0xe5,0xe9,0x61,0x47,0x2f,0x35,0x7f,0x17,0x7f,0xef,0x7f,0x95,0x95,0x71,0xd3,0xa3,
    0x0b,0x71,0xa3,0xad,0x0b,0x3b,0xb5,0xfb,0xa3,0xbf,0x4f,0x83,0x1d,0xad,0xe9,0x2f,
    0x71,0x65,0xa3,0xe5,0x07,0x35,0x3d,0x0d,0xb5,0xe9,0xe5,0x47,0x3b,0x9d,0xef,0x35,
    0xa3,0xbf,0xb3,0xdf,0x53,0xd3,0x97,0x53,0x49,0x71,0x07,0x35,0x61,0x71,0x2f,0x43,
    0x2f,0x11,0xdf,0x17,0x97,0xfb,0x95,0x3b,0x7f,0x6b,0xd3,0x25,0xbf,0xad,0xc7,0xc5,
    0xc5,0xb5,0x8b,0xef,0x2f,0xd3,0x07,0x6b,0x25,0x49,0x95,0x25,0x49,0x6d,0x71,0xc7 },
  { 0xa7,0xbc,0xc9,0xad,0x91,0xdf,0x85,0xe5,0xd4,0x78,0xd5,0x17,0x46,0x7c,0x29,0x4c,
    0x4d,0x03,0xe9,0x25,0x68,0x11,0x86,0xb3,0xbd,0xf7,0x6f,0x61,0x22,0xa2,0x26,0x34,
    0x2a,0xbe,0x1e,0x46,0x14,0x68,0x9d,0x44,0x18,0xc2,0x40,0xf4,0x7e,0x5f,0x1b,0xad,
    0x0b,0x94,0xb6,0x67,0xb4,0x0b,0xe1,0xea,0x95,0x9c,0x66,0xdc,0xe7,0x5d,0x6c,0x05,
    0xda,0xd5,0xdf,0x7a,0xef,0xf6,0xdb,0x1f,0x82,0x4c,0xc0,0x68,0x47,0xa1,0xbd,0xee,
    0x39,0x50,0x56,0x4a,0xdd,0xdf,0xa5,0xf8,0xc6,0xda,0xca,0x90,0xca,0x01,0x42,0x9d,
    0x8b,0x0c,0x73,0x43,0x75,0x05,0x94,0xde,0x24,0xb3,0x80,0x34,0xe5,0x2c,0xdc,0x9b,
    0x3f,0xca,0x33,0x45,0xd0,0xdb,0x5f,0xf5,0x52,0xc3,0x21,0xda,0xe2,0x22,0x72,0x6b,
    0x3e,0xd0,0x5b,0xa8,0x87,0x8c,0x06,0x5d,0x0f,0xdd,0x09,0x19,0x93,0xd0,0xb9,0xfc,
    0x8b,0x0f,0x84,0x60,0x33,0x1c,0x9b,0x45,0xf1,0xf0,0xa3,0x94,0x3a,0x12,0x77,0x33,
    0x4d,0x44,0x78,0x28,0x3c,0x9e,0xfd,0x65,0x57,0x16,0x94,0x6b,0xfb,0x59,0xd0,0xc8,
    0x22,0x36,0xdb,0xd2,0x63,0x98,0x43,0xa1,0x04,0x87,0x86,0xf7,0xa6,0x26,0xbb,0xd6,
    0x59,0x4d,0xbf,0x6a,0x2e,0xaa,0x2b,0xef,0xe6,0x78,0xb6,0x4e,0xe0,0x2f,0xdc,0x7c,
    0xbe,0x57,0x19,0x32,0x7e,0x2a,0xd0,0xb8,0xba,0x29,0x00,0x3c,0x52,0x7d,0xa8,0x49,
    0x3b,0x2d,0xeb,0x25,0x49,0xfa,0xa3,0xaa,0x39,0xa7,0xc5,0xa7,0x50,0x11,0x36,0xfb,
    0xc6,0x67,0x4a,0xf5,0xa5,0x12,0x65,0x7e,0xb0,0xdf,0xaf,0x4e,0xb3,0x61,0x7f,0x2f } };
  uchar ck=0x60;

  // if (strncmp ((char *)buf, "02", 2)) return;
  if ( start >= size ) return ;

  ci = xlat[0][ci];
  cj = xlat[1][cj];
//printf("Decrypted tag 0x%x:\n%*s", tag, (i & 31)*3, "");
  for (uint64_t i = start ; i < size; i++) {
  // printf("%02x%c", buf[i] ^ (cj += ci * ck++), (i & 31) == 31 ? '\n':' ');
    buf[i] ^= (cj += ci * ck++);
  }
  // if (size & 31) puts("");
}

// types of data in Exif Specification
enum type_e
{    kttMin             = 0
,    kttUByte           = 1 //!< Exif BYTE type, 8-bit unsigned integer.
,    kttAscii           = 2 //!< Exif ASCII type, 8-bit byte.
,    kttUShort          = 3 //!< Exif SHORT type, 16-bit (2-byte) unsigned integer.
,    kttULong           = 4 //!< Exif LONG type, 32-bit (4-byte) unsigned integer.
,    kttURational       = 5 //!< Exif RATIONAL type, two LONGs: numerator and denominator of a fraction.
,    kttByte            = 6 //!< Exif SBYTE type, an 8-bit signed (twos-complement) integer.
,    kttUndefined       = 7 //!< Exif UNDEFINED type, an 8-bit byte that may contain anything.
,    kttShort           = 8 //!< Exif SSHORT type, a 16-bit (2-byte) signed (twos-complement) integer.
,    kttLong            = 9 //!< Exif SLONG type, a 32-bit (4-byte) signed (twos-complement) integer.
,    kttSRational       =10 //!< Exif SRATIONAL type, two SLONGs: numerator and denominator of a fraction.
,    kttFloat           =11 //!< TIFF FLOAT type, single precision (4-byte) IEEE format.
,    kttDouble          =12 //!< TIFF DOUBLE type, double precision (8-byte) IEEE format.
,    kttIfd             =13 //!< TIFF IFD type, 32-bit (4-byte) unsigned integer.
,    kttNot1            =14
,    kttNot2            =15
,    kttULong8          =16 //!< Exif LONG LONG type, 64-bit (8-byte) unsigned integer.
,    kttLong8           =17 //!< Exif LONG LONG type, 64-bit (8-byte) signed integer.
,    kttIfd8            =18 //!< TIFF IFD type, 64-bit (8-byte) unsigned integer.
,    kttMax             =19
};
const char* typeName(type_e tag)
{
    //! List of TIFF image tags
    const char* result = NULL;
    switch (tag ) {
        case kttUByte      : result = "UBYTE"     ; break;
        case kttAscii      : result = "ASCII"     ; break;
        case kttUShort     : result = "SHORT"     ; break;
        case kttULong      : result = "LONG"      ; break;
        case kttURational  : result = "RATIONAL"  ; break;
        case kttByte       : result = "BYTE"      ; break;
        case kttUndefined  : result = "UNDEFINED" ; break;
        case kttShort      : result = "SSHORT"    ; break;
        case kttLong       : result = "SLONG"     ; break;
        case kttSRational  : result = "SRATIONAL" ; break;
        case kttFloat      : result = "FLOAT"     ; break;
        case kttDouble     : result = "DOUBLE"    ; break;
        case kttIfd        : result = "IFD"       ; break;
        case kttULong8     : result = "LONG8"     ; break;
        case kttLong8      : result = "LONG8"     ; break;
        case kttIfd8       : result = "IFD8"      ; break;
        default            : result = "unknown"   ; break;
    }
    return result;
}

enum endian_e
{   keLittle
,   keBig
,   keImage // used by a field  to say "use image's endian"
};

// Error support
enum error_e
{   kerCorruptedMetadata
,   kerTiffDirectoryTooLarge
,   kerInvalidTypeValue
,   kerInvalidMalloc
,   kerInvalidFileFormat
,   kerFailedToReadImageData
,   kerDataSourceOpenFailed
,   kerNoImageInInputData
,   kerFileDidNotOpen
,   kerUnknownFormat
,   kerAlreadyVisited
,   kerInvalidMemoryAccess
};

std::string error_program;
std::string error_path   ;
void Error (error_e error, std::string msg,const std::string& m2="")
{
    if ( !isatty(STDIN_FILENO))  std::cerr << error_program << " " << error_path << ": ";
    switch ( error ) {
        case   kerCorruptedMetadata      : std::cerr << "corrupted metadata"       ; break;
        case   kerTiffDirectoryTooLarge  : std::cerr << "tiff directory too large" ; break;
        case   kerInvalidTypeValue       : std::cerr << "invalid type"             ; break;
        case   kerInvalidMalloc          : std::cerr << "invalid malloc"           ; break;
        case   kerInvalidFileFormat      : std::cerr << "invalid file format"      ; break;
        case   kerFailedToReadImageData  : std::cerr << "failed to read image data"; break;
        case   kerDataSourceOpenFailed   : std::cerr << "data source open failed"  ; break;
        case   kerNoImageInInputData     : std::cerr << "not image in input data"  ; break;
        case   kerFileDidNotOpen         : std::cerr << "file did not open"        ; break;
        case   kerUnknownFormat          : std::cerr << "unknown format " << m2    ; break;
        case   kerAlreadyVisited         : std::cerr << "already visited"          ; break;
        case   kerInvalidMemoryAccess    : std::cerr << "invalid memory access"    ; break;
        default                          : std::cerr << "unknown error"            ; break;
    }
    if ( msg.size() ) std::cerr << " " << msg ;
    std::cerr << std::endl;
    _exit(1); // pull the plug!
}

void Error (error_e error, size_t n)
{
    std::ostringstream os ;
    os << n ;
    Error(error,os.str());
}

void Error (error_e error)
{
    Error(error,"");
}
// forward and prototypes
class Io;
class DataBuf ; // forward
typedef unsigned char byte ;
uint8_t  getByte (const DataBuf& buf,size_t offset) ;
uint16_t getShort(const DataBuf& buf,size_t offset,endian_e endian);
uint32_t getLong (const DataBuf& buf,size_t offset,endian_e endian);
uint64_t getLong8(const DataBuf& buf,size_t offset,endian_e endian);

class DataBuf
{
public:
    byte*     pData_;
    uint64_t  size_ ;
    DataBuf(uint64_t size=0,uint64_t size_max=0)
    : pData_(NULL)
    , size_(size)
    {
        if ( size ) {
            if ( size_max && size > size_max ) {
                Error(kerInvalidMalloc);
            }
            pData_ = (byte*) std::calloc(size_,1);
        }
    }
    DataBuf(DataBuf& other)
    : pData_(NULL)
    , size_ (0)
    {
        if ( other.size_ ) {
            malloc(other.size_);
            memcpy(pData_,other.pData_,size_);
        }
    }
    virtual ~DataBuf()
    {
        empty(true);
    }
    bool empty(bool bForce=false) {
        bool result = size_ == 0;

        if ( bForce && pData_ && size_ ) {
            std::free(pData_) ;
            pData_ = NULL ;
        }
        if ( bForce ) size_ = 0;
        return result;
    }
    void malloc(uint64_t size)
    {
        empty(true);
        pData_ = (byte*) std::malloc(size_);
        if ( pData_ ) {
            size_  = size ;
            ::memset(pData_,0,size_);
        }
    }

    void read (Io& io,uint64_t offset,uint64_t size) ;
    int  strcmp   (const char* str) { return ::strcmp((const char*)pData_,str);}
    bool strequals(const char* str) { return strcmp(str)==0                   ;}
    bool begins       (const char* str,uint64_t offset=0) {
        uint64_t l      = ::strlen(str);
        bool     result = l <= size_-offset;
        size_t   i = 0 ;
        while ( result && i < l ) {
            result = str[i]==pData_[i+offset];
            i++;
        }
        return result;
    }
    void  zero() { for ( uint64_t i = 0 ; i < size_ ; i++ ) pData_[i]=0; }
    char* getChars(uint64_t offset,uint64_t count,char* chars) {
        chars[0] = 0;
        if ( offset + count <= size_ ) {
            for ( uint64_t i = 0 ; i < count ; i++ )
                chars[i] = (char) pData_[offset+i];
            chars[count] = 0;
        }
        return chars;
    }
    uint8_t  getByte (uint64_t offset                ) { return ::getByte (*this,offset)       ; }
    uint16_t getShort(uint64_t offset,endian_e endian) { return ::getShort(*this,offset,endian); }
    uint32_t getLong (uint64_t offset,endian_e endian) { return ::getLong (*this,offset,endian); }
    uint64_t getLong8(uint64_t offset,endian_e endian) { return ::getLong8(*this,offset,endian); }
    uint32_t search(uint32_t start,const char* s)
    {
        uint64_t l = ::strlen(s);
        if ( size_ < l ) return 0 ;

        uint32_t found = start;
        uint32_t max   = (uint32_t) ( size_ - l );
        while ( ! begins(s,found) && found < max ) found++;
        return found;
    }
    bool hasNull()
    {
        for (size_t i = 0 ; i < size_ ; i++ )
            if ( !pData_[i] )
                return true ;
        return false;
    }
    void copy(void* src,uint64_t size,uint64_t offset=0)
    {
        memcpy(pData_+offset,src,size);
    }
    void copy(uint32_t v,uint64_t offset=0) { copy(&v,4,offset); }
    void copy(DataBuf& src,uint64_t size=0) {
        if ( !size ) size=src.size_;
        malloc(size) ;

        if (size <= src.size_ && size <= size_) {
            memcpy(pData_,src.pData_,size);
        }
    }
    void append(DataBuf& src,bool bEmpty=false) {
        if ( bEmpty ) empty(bEmpty);
        if ( src.size_ ) {
            pData_ = pData_ ? (byte*) std::realloc(pData_,size_+src.size_)
                            : (byte*) std::malloc(src.size_)
            ;
            std::memcpy(pData_+size_,src.pData_,src.size_); // copy from src
            size_ += src.size_;                             // update size
        }
    }
    std::string path() const { return path_; }
    std::string toString(type_e type,uint64_t count,endian_e endian,uint64_t offset=0) const;
    std::string binaryToString(uint64_t start,uint64_t size) const;
    std::string toUuidString(uint64_t offset=0) const ;
    std::string toHexString (uint64_t offset=0,uint64_t count=0) const;

private:
    std::string path_;
};

// endian and byte swappers
bool isPlatformBigEndian()
{
    union {
        uint32_t i;
        char c[4];
    } e = { 0x01000000 };
    return e.c[0]?true:false;
}
bool   isPlatformLittleEndian() { return !isPlatformBigEndian(); }
endian_e platformEndian() { return isPlatformBigEndian() ? keBig : keLittle; }

uint64_t byteSwap(byte* p,bool bSwap,uint16_t n)
{
    uint64_t value  = 0;
    uint64_t result = 0;
    byte* source_value      = reinterpret_cast<byte *>(&value );
    byte* destination_value = reinterpret_cast<byte *>(&result);

    for (int i = 0; i < n; i++) {
        source_value     [i] = p[i] ;
        destination_value[i] = p[n - i - 1];
    }

    return bSwap ? result : value;
}

uint8_t  getByte(const DataBuf& buf,size_t offset)
{
    if ( offset > buf.size_ ) Error(kerInvalidMemoryAccess);
    return (uint8_t) buf.pData_[offset];
}

uint16_t getShort(byte b[],size_t offset,endian_e endian)
{
    bool bSwap = endian != ::platformEndian();
    return (uint16_t) byteSwap(b+offset,bSwap,2);
}
uint32_t getLong(byte b[],size_t offset,endian_e endian)
{
    bool bSwap = endian != ::platformEndian();
    return (uint32_t) byteSwap(b+offset,bSwap,4);
}
uint64_t getLong8(byte b[],size_t offset,endian_e endian)
{
    bool bSwap = endian != ::platformEndian();
    return (uint64_t) byteSwap(b+offset,bSwap,8);
}

uint16_t getPascalStringLength(DataBuf& buff,uint32_t offset)
{
    uint8_t  L = buff.pData_[offset];  // #abc
    uint16_t l = L==0  ? 2
               : L % 2 ? L + 1
               : L     ;
    return l;
}

uint16_t getShort(const DataBuf& buf,size_t offset,endian_e endian)
{
    if ( offset+1 > buf.size_ ) Error(kerInvalidMemoryAccess);

    uint16_t v;
    byte*    p = (byte*) &v;
    p[0] = buf.pData_[offset+0];
    p[1] = buf.pData_[offset+1];
    bool bSwap = endian != ::platformEndian();
    return (uint16_t) byteSwap(p,bSwap,2);

}
uint32_t getLong(const DataBuf& buf,size_t offset,endian_e endian)
{
    if ( offset+3 > buf.size_ ) Error(kerInvalidMemoryAccess);

    uint32_t v;
    byte*    p = (byte*) &v;
    p[0] = buf.pData_[offset+0];
    p[1] = buf.pData_[offset+1];
    p[2] = buf.pData_[offset+2];
    p[3] = buf.pData_[offset+3];
    bool bSwap = endian != ::platformEndian();
    return (uint32_t)byteSwap(p,bSwap,4);
}
uint64_t getLong8(const DataBuf& buf,size_t offset,endian_e endian)
{
    if ( offset+7 > buf.size_ ) Error(kerInvalidMemoryAccess);

    uint64_t v;
    byte*    p = reinterpret_cast<byte *>(&v);
    p[0] = buf.pData_[offset+0];
    p[1] = buf.pData_[offset+1];
    p[2] = buf.pData_[offset+2];
    p[3] = buf.pData_[offset+3];
    p[4] = buf.pData_[offset+4];
    p[5] = buf.pData_[offset+5];
    p[6] = buf.pData_[offset+6];
    p[7] = buf.pData_[offset+7];
    bool bSwap = endian != ::platformEndian();
    return byteSwap (p,bSwap,8);
}

// Tiff Data Functions
bool isTypeShort(type_e type) {
     return type == kttUShort
         || type == kttShort
         ;
}
bool isTypeLong(type_e type) {
     return type == kttULong
         || type == kttLong
         || type == kttIfd
         || type == kttFloat
         ;
}
bool isTypeLong8(type_e type) {
    return type == kttULong8
        || type == kttLong8
        || type == kttIfd8
        || type == kttDouble
        ;
}
bool isTypeRational(type_e type) {
     return type == kttURational
         || type == kttSRational
         ;
}
bool isTypeIFD(type_e type)
{
    return type == kttIfd || type == kttIfd8;
}
bool isType1Byte(type_e type)
{
    return type == kttAscii
        || type == kttUByte
        || type == kttByte
        || type == kttUndefined
        ;
}
bool isType2Byte(type_e type)
{
    return isTypeShort(type);
}
bool isType4Byte(type_e type)
{
    return isTypeLong(type)
        || type == kttFloat
        ;
}
bool isType8Byte(type_e type)
{
    return  isTypeRational(type)
         || isTypeLong8(type)
         || type == kttIfd8
         || type == kttDouble
         ;
}
uint16_t typeSize(type_e type)
{
    return isType1Byte(type) ? 1
        :  isType2Byte(type) ? 2
        :  isType4Byte(type) ? 4
        :  isType8Byte(type) ? 8
        :  1 ;
}
type_e getType(const DataBuf& buf,size_t offset,endian_e endian)
{
    return (type_e) getShort(buf,offset,endian);
}
bool typeValid(type_e type,bool bigtiff)
{
    return  bigtiff ? type > kttMin && type < kttMax && type != kttNot1 && type != kttNot2
                    : type >= 1 && type <= 13
    ;
}

// string formatting functions
std::string indent(size_t s)
{
    std::string result ;
    while ( s-- > 1) result += "  ";
    return result ;
}

// chop("a very long string",10) -> "a ver +++"
std::string chop(const std::string& a,size_t max=0)
{
    std::string result = a;
    if ( result.size() > max  && max > 4 ) {
        result = result.substr(0,max-4) + " +++";
    }
    return result;
}

// snip("a very long string",10) -> "a very lo"
std::string snip(const std::string& a,size_t max=0)
{
    std::string result = a;
    if ( result.size() > max  && max) {
        result = result.substr(0,max);
    }
    return result;
}

// join("Exif.Nikon","PictureControl",22) -> "Exif.Nikon.PictureC.."
std::string join(const std::string& a,const std::string& b,size_t max=0)
{
    std::string c = a + "." +  b ;
    if ( max > 2 && c.size() > max ) {
        c = c.substr(0,max-2)+"..";
    }
    return c;
}

std::string stringFormat(const char* format, ...)
{
    std::string result;
    std::vector<char> buffer;
    size_t need = ::strlen(format)*8;  // initial guess
    int rc = -1;

    // vsnprintf writes at most size (2nd parameter) bytes (including \0)
    //           returns the number of bytes required for the formatted string excluding \0
    // the following loop goes through:
    // one iteration (if 'need' was large enough for the for formatted string)
    // or two iterations (after the first call to vsnprintf we know the required length)
    do {
        buffer.resize(need + 1);
        va_list args;            // variable arg list
        va_start(args, format);  // args start after format
        rc = vsnprintf(&buffer[0], buffer.size(), format, args);
        va_end(args);     // free the args
        assert(rc >= 0);  // rc < 0 => we have made an error in the format string
        if (rc > 0)
            need = static_cast<size_t>(rc);
    } while (buffer.size() <= need);

    if (rc > 0)
        result = std::string(&buffer[0], need);
    return result;
}

std::string binaryToString(const byte* b,uint64_t start,uint64_t size)
{
    std::string result;
    size_t i    = start;
    while (i < start+size ) {
        result += (32 <= b[i] && b[i] <= 127) ? b[i]
                : ( 0 == b[i]               ) ? '_'
                : '.'
                ;
        i++ ;
    }
    return result;
}

// https://stackoverflow.com/questions/11635/case-insensitive-string-comparison-in-c
bool isEqualsNoCase(const std::string& a, const std::string& b)
{
    return std::equal(a.begin(), a.end(), b.begin(),
                      [](char a, char b) {
                          return tolower(a) == tolower(b);
                      });
}

std::string DataBuf::binaryToString(uint64_t start=0,uint64_t size=0) const
{
    return ::binaryToString(pData_,start,size?size:size_);
}

std::string DataBuf::toString(type_e type,uint64_t count=0,endian_e endian=keLittle,uint64_t offset/*=0*/) const
{
    std::ostringstream os;
    std::string        sp;
    uint16_t           size = typeSize(type);
    if ( !count )     count = size_/typeSize(type);
    if ( isTypeShort(type) ){
        for ( uint64_t k = 0 ; k < count ; k++ ) {
            os << sp << ::getShort(*this,offset+k*size,endian);
            sp = " ";
        }
    } else if ( isTypeLong(type) ){
        for ( uint64_t k = 0 ; k < count ; k++ ) {
            os << sp << ::getLong(*this,offset+k*size,endian);
            sp = " ";
        }
    } else if ( isTypeRational(type) ){
        for ( uint64_t k = 0 ; k < count ; k++ ) {
            uint32_t a = ::getLong(*this,offset+k*size+0,endian);
            uint32_t b = ::getLong(*this,offset+k*size+4,endian);
            os << sp << a << "/" << b;
            sp = " ";
        }
    } else if ( isType8Byte(type) ) {
        for ( uint64_t k = 0 ; k < count ; k++ ) {
            os << sp << ::getLong8(*this,offset+k*size,endian);
            sp = " ";
        }
    } else if ( type == kttUByte ) {
        for ( size_t k = 0 ; k < count ; k++ )
            os << stringFormat("%s%d",k?" ":"",pData_[offset+k]);
    } else if ( type == kttAscii ) {
        bool bNoNull = true ;
        for ( size_t k = 0 ; bNoNull && k < count ; k++ )
            bNoNull = pData_[offset+k];
        if ( bNoNull )
            os << binaryToString(offset, (size_t)count);
        else
            os << (char*) pData_+offset ;
    } else {
        os << sp << binaryToString(offset, (size_t)count);
    }

    return os.str();
} // DataBuf::toString

std::string DataBuf::toUuidString(uint64_t offset /* =0 */) const
{
    // 123e4567-e89b-12d3-a456-426614174000
    std::string result ;
    if ( size_ >= 16+offset ) {
        byte* p = pData_+offset;
        result = ::stringFormat("%02x%02x%02x%02x-"        "%02x%02x-"      "%02x%02x-"
                                "%02x%02x-"                "%02x%02x%02x%02x%02x%02x"
                                ,p[ 0],p[ 1],p[ 2],p[ 3]   ,p[ 4],p[ 5]     ,p[ 6],p[ 7]
                                ,p[ 8],p[ 9]               ,p[10],p[11],p[12],p[13],p[14],p[15]
                               );
    }
    return result ;
} // DataBuf::toUuidString

std::string DataBuf::toHexString(uint64_t offset /*=0*/,uint64_t count /*=0*/) const
{
    std::string result;
    if ( !count ) count = size_ - offset ;
    for ( uint64_t i = offset ; i < offset+count ; i++ ) {
        result+=stringFormat("%02x",pData_[i]);
    }
    return result;
} // DataBuf::toHexString

class ExifDatum {
public:
    ExifDatum(uint64_t address,std::string name, uint16_t tag,type_e type,uint32_t count,uint64_t offset,DataBuf& buff)
    : address_(address)
    , name_   (name)
    , tag_    (tag)
    , count_  (count)
    , offset_ (offset)
    , buff_   (buff)
    {}
private:
    uint64_t    address_;
    std::string name_   ;
    uint16_t    tag_    ;
    type_e      type_   ;
    uint32_t    count_  ;
    uint64_t    offset_ ;
    DataBuf     buff_   ;
};

// Camera makers
enum maker_e
{   kUnknown
,   kCanon
,   kNikon
,   kSony
,   kAgfa
,   kApple
,   kPana
,   kMino
,   kFuji
,   kOlym
,   kPentax
};
typedef std::map<std::string,maker_e> Maker_t;
typedef Maker_t::iterator             Maker_i;
Maker_t makers;

// Canon magic
enum kCanonHeap
{   kStg_InHeapSpace    = 0
,   kStg_InRecordEntry  = 0x4000
};
// Canon tag masks
#define kcAscii        0x0800
#define kcWord         0x1000
#define kcDword        0x1000
#define kcHTP1         0x2800
#define kcHTP2         0x3000
#define kcIDCodeMask   0x07ff
#define kcDataTypeMask 0x3800
enum kCanonType
{   kDT_BYTE            = 0x0000
,   kDT_ASCII           = kcAscii
,   kDT_WORD            = kcWord
,   kDT_DWORD           = kcDword
,   kDT_BYTE2           = 0x2000
,   kDT_HeapTypeProp1   = kcHTP1
,   kDT_HeapTypeProp2   = kcHTP1
,   kTC_WildCard        = 0xffff
,   kTC_Null            = 0x0000
,   kTC_Free            = 0x0001
,   kTC_ExUsed          = 0x0002
,   kTC_Description     = 0x0005|kcAscii
,   kTC_ModelName       = 0x000a|kcAscii
,   kTC_FirmwareVersion = 0x000b|kcAscii
,   kTC_ComponentVersion= 0x000c|kcAscii
,   kTC_ROMOperationMode= 0x000d|kcAscii
,   kTC_OwnerName       = 0x0010|kcAscii
,   kTC_ImageFileName   = 0x0016|kcAscii
,   kTC_ThumbnailFileName=0x001c|kcAscii
,   kTC_TargetImageType = 0x000a|kcWord
,   kTC_SR_ReleaseMethod= 0x0010|kcWord
,   kTC_SR_ReleaseTiming= 0x0011|kcWord
,   kTC_ReleaseSetting  = 0x0016|kcWord
,   kTC_BodySensitivity = 0x001c|kcWord
,   kTC_ImageFormat     = 0x0003|kcDword
,   kTC_RecordID        = 0x0004|kcDword
,   kTC_SelfTimerTime   = 0x0006|kcDword
,   kTC_SR_TargetDistanceSetting = 0x0007|kcDword
,   kTC_BodyID          = 0x000b|kcDword
,   kTC_CapturedTime    = 0x000e|kcDword
,   kTC_ImageSpec       = 0x0010|kcDword
,   kTC_SR_EF           = 0x0013|kcDword
,   kTC_MI_EV           = 0x0014|kcDword
,   kTC_SerialNumber    = 0x0017|kcDword
,   kTC_SR_Exposure     = 0x0018|kcDword
,   kTC_CameraObject    = 0x0007|kcHTP1
,   kTC_ShootingRecord  = 0x0002|kcHTP2
,   kTC_MeasuredInfo    = 0x0003|kcHTP2
,   kTC_CameraSpecification= 0x0004|kcHTP2
};

// TagDict is used to map tag (uint16_t) to string
typedef std::map<uint16_t,std::string> TagDict;
TagDict emptyDict ;
TagDict tiffDict  ;
TagDict dngDict   ;
TagDict exifDict  ;
TagDict canonDict ;
TagDict nikonDict ;
TagDict sonyDict  ;
TagDict agfaDict  ;
TagDict appleDict ;
TagDict panaDict  ;
TagDict minoDict  ;
TagDict fujiDict  ;
TagDict olymDict  ;
TagDict gpsDict   ;
TagDict crwDict   ;
TagDict psdDict   ;
TagDict olymCSDict;
TagDict olymEQDict;
TagDict olymRDDict;
TagDict olymR2Dict;
TagDict olymIPDict;
TagDict olymFIDict;
TagDict olymRoDict;
TagDict pentaxDict;

std::map<maker_e,TagDict*> makerDicts;

enum ktSpecial
{   ktMN        = 0x927c
,   ktGps       = 0x8825
,   ktExif      = 0x8769
,   ktIOP       = 0xa005
,   ktSubIFD    = 0x014a
,   ktMake      = 0x010f
,   ktXML       = 0x02bc
,   ktIPTC      = 0x83bb
,   ktIPTCPS    = 0x0404
,   ktICC       = 0x8773
,   ktMNP       = 0xc634 // Pentax MakerNote and DNGPrivateData
,   ktGroup     = 0xffff
};

std::map<uint16_t,TagDict> iptcDicts;
TagDict iptc0 ;
TagDict iptcEnvelope;
TagDict iptcApplication;

TagDict& ifdDict(maker_e maker,uint16_t tag,TagDict& makerDict)
{
    TagDict& result = tag == ktExif ? exifDict
                    : tag == ktGps  ? gpsDict
                    : makerDict ;
    if ( maker == kOlym ) switch ( tag ) {
        case 0x2010 : result = olymEQDict ; break;
        case 0x2020 : result = olymCSDict ; break;
        case 0x2030 : result = olymRDDict ; break;
        case 0x2031 : result = olymR2Dict ; break;
        case 0x2040 : result = olymIPDict ; break;
        case 0x2050 : result = olymFIDict ; break;
        case 0x3000 : result = olymRoDict ; break;
        default     : /* do nothing */    ; break;
    }
    return result;
}

bool tagKnown(uint16_t tag,const TagDict& tagDict)
{
    return tagDict.find(tag) != tagDict.end();
}

std::string groupName(const TagDict& tagDict,std::string family="Exif" )
{
    std::string group = tagKnown(ktGroup,tagDict)
                      ? tagDict.find(ktGroup)->second
                      : "Unknown"
                      ;
    return family+ "." + group ;
}

std::string tagName(uint16_t tag,const TagDict& tagDict,const size_t max=0,std::string family="Exif")
{
    // prioritize dngDict above tiffDict
    bool bTagDict    = ! (tagKnown(tag,dngDict) && tagDict == tiffDict);
    std::string name = tagKnown(tag,bTagDict?tagDict:dngDict)
                     ? (bTagDict?tagDict.find(tag)->second:dngDict.find(tag)->second)
                     : stringFormat("%#x",tag)
                     ;
    name =  groupName(bTagDict?tagDict:dngDict,family) + "." + name;
    if ( max && name.size() > max ){
        name = name.substr(0,max-2)+"..";
    }
    return name;
}

// Binary Records
class Field
{
public:
    Field
    ( std::string name
    , type_e      type
    , uint16_t    start
    , uint16_t    count
    , endian_e    endian = keImage
    )
    : name_  (name)
    , type_  (type)
    , start_ (start)
    , count_ (count)
    , endian_(endian)
    {};
    virtual ~Field() {}
    std::string name  () { return name_   ; }
    type_e      type  () { return type_   ; }
    uint16_t    start () { return start_  ; }
    uint16_t    count () { return count_  ; }
    endian_e    endian() { return endian_ ; }
private:
    std::string name_   ;
    type_e      type_   ;
    uint16_t    start_  ;
    uint16_t    count_  ;
    endian_e    endian_ ;
};
typedef std::vector<Field>   Fields;
typedef std::map<std::string,Fields>      MakerTags;
typedef std::map<std::string,std::string> Dict;

// global variables
MakerTags makerTags;
MakerTags boxTags  ;
Dict      boxDict ;

// https://github.com/openSUSE/libsolv/blob/master/win32/fmemopen.c
#if defined(_MSC_VER) || defined(__MINGW64__)
FILE* fmemopen(void* buf, size_t size, const char* mode)
{
    char temppath[MAX_PATH + 1];
    char tempnam[MAX_PATH + 1];
    DWORD l;
    HANDLE fh;
    FILE* fp;

    if (strcmp(mode, "r") != 0 && strcmp(mode, "r+") != 0)
        return 0;
    l = GetTempPath(MAX_PATH, temppath);
    if (!l || l >= MAX_PATH)
        return 0;
    if (!GetTempFileName(temppath, "solvtmp", 0, tempnam))
        return 0;
    fh = CreateFile(tempnam, DELETE | GENERIC_READ | GENERIC_WRITE, 0,
        NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
        NULL);
    if (fh == INVALID_HANDLE_VALUE)
        return 0;
    fp = _fdopen(_open_osfhandle((intptr_t)fh, 0), "w+b");
    if (!fp)
    {
        CloseHandle(fh);
        return 0;
    }
    if (buf && size && fwrite(buf, size, 1, fp) != 1)
    {
        fclose(fp);
        return 0;
    }
    rewind(fp);
    return fp;
}
#endif

// IO support
enum seek_e
{   ksStart   = SEEK_SET
,   ksCurrent = SEEK_CUR
,   ksEnd     = SEEK_END
};
class Io
{
public:
    Io(std::string path,std::string open) // Io object from path
    : path_   (path)
    , start_  (0)
    , size_   (0)
    , restore_(0)
    , f_      (NULL)
    { f_ = ::fopen(path.c_str(),open.c_str()); if ( !f_ ) Error(kerFileDidNotOpen,path); }

    Io(Io& io,size_t start,size_t size=0) // Io object is a substream
    : path_   (io.path())
    , start_  (start+io.start_)
    , size_   (size?size:io.size()-start)
    , restore_(ftell(f_))
    , f_      (io.f_)
    {
        std::ostringstream os;
        os << path_ << ":" << start << "->" << size_;
        path_=os.str();
        seek(0);
    };
    Io(DataBuf& buf)
    : path_   (buf.path())
    , start_  (0)
    , size_   (buf.size_)
    , restore_(0)
    , f_      (NULL)
    {   f_ = fmemopen(buf.pData_,buf.size_, "r");
        if ( !f_ ) Error(kerFileDidNotOpen,path_);
    }

    virtual ~Io() { close(); }
    std::string path() { return path_; }
    uint64_t read(void* buff,uint64_t size)              { return fread(buff,1,size,f_);}
    uint64_t read(DataBuf& buff)                         { return read(buff.pData_,buff.size_); }
    byte     getb()                                      { byte b; if (read(&b,1)==1) return b ; else return -1; }
    byte     peek()                                      { uint64_t a = tell() ; byte result = getb() ; seek(a) ; return result ; }
    int      eof()                                       { return feof(f_) ; }
    uint64_t tell()                                      { return ftell(f_)-start_ ; }
    void     seek(int64_t offset,seek_e whence=ksStart)  { fseek(f_,(FSEEK_LONG)(offset+start_),whence) ; }
    uint64_t size()                                      { if ( size_ ) return size_ ; struct stat st ; fstat(::fileno(f_),&st) ; return st.st_size-start_ ; }
    bool     good()                                      { return f_ ? true : false ; }
    void     close()
    {
        if ( !f_ ) return ;
        if ( start_ == 0 && size_ == 0 && restore_ == 0 ) {
            fclose(f_) ;
        } else {
            fseek(f_,(FSEEK_LONG)restore_,ksStart);
        }
        f_ = NULL  ;
    }
    uint64_t start() { return start_ ; }

    uint32_t getLong(endian_e endian)
    {
        DataBuf buf(4);
        read   (buf);
        return ::getLong(buf,0,endian);
    }
    uint64_t getLong8(endian_e endian)
    {
        DataBuf buf(8);
        read   (buf);
        return ::getLong8(buf,0,endian);
    }
    float getFloat(endian_e endian)
    {
        return (float) getLong(endian);
    }
    uint16_t getShort(endian_e endian)
    {
        byte b[2];
        read(b,2);
        return ::getShort(b,0,endian);
    }
    uint8_t getByte()
    {
        byte   b[1];
        read  (b,1);
        return b[0];
    }
private:
    FILE*       f_;
    std::string path_;
    uint64_t    start_;
    uint64_t    size_;
    uint64_t    restore_;
};

class IoSave // restore Io when function ends
{
public:
    IoSave(Io& io,uint64_t address)
    : io_     (io)
    , restore_(io.tell())
    { io_.seek(address); }
    IoSave(Io& io)
    : io_     (io)
    , restore_(io.tell())
    {}
    virtual ~IoSave() {io_.seek(restore_);}
private:
    Io&      io_;
    uint64_t restore_;
};

void DataBuf::read(Io& io,uint64_t offset,uint64_t size)
{
    if ( size ) {
        IoSave restore(io,offset);
        pData_ = (byte*) (pData_ ? std::realloc(pData_,size_+size) : std::malloc(size));
        if ( !path_.size() ) path_=io.path();
        std::ostringstream os ;
        os <<"+"<<io.tell()<<"->"<<size;
        path_ += os.str();
        io.read (pData_+size_,size);
        size_ += size ;
    }
}

// Options for ReportVisitor
typedef uint16_t PSOption;
#define kpsRecursive 2
#define kpsUnknown   8

// 1.  declare types
class   Image; // forward
class   TiffImage;

// 2. Create abstract "visitor" base class with virtual methods
class Visitor
{
public:
    Visitor(std::ostream& out,PSOption option)
    : out_   (out)
    , option_(option)
    , indent_(0)
    {};
    virtual ~Visitor() {};

    // abstract methods which visitors must define
    virtual void visitBegin   (Image& image,std::string msg="")      = 0 ;
    virtual void visitEnd     (Image& image)                         = 0 ;
    virtual void visitDirBegin(Image& image,uint64_t nEntries)       = 0 ;
    virtual void visitDirEnd  (Image& image,uint64_t start)          = 0 ;
    virtual void visitTag     (Io& io,Image& image
                        ,uint64_t address, uint16_t tag, type_e type
                        ,uint32_t count,   uint64_t offset
                        ,DataBuf& buf,     const TagDict& tagDict  ) = 0 ;
    virtual void visitExif    (Io& io)                               = 0 ;

    // optional methods
    virtual void showError    (std::string message )                                 { return ; }

    PSOption      option() { return option_ ; }
    std::ostream& out()    { return out_    ; }
    std::string   indent(uint32_t i=0) { return ::indent(indent_+i);}
    bool          isRecursive()        { return (option_ & kpsRecursive  ) ? true : false;}
    bool          isBasicOrRecursive() { return isRecursive(); }
protected:
    uint32_t      indent_ ;
    PSOption      option_;
    std::ostream& out_   ;
};

// 3. Image has an accept(Visitor&) method
class Image
{
public:
    Image(std::string path)
    : io_(path,"rb")
    , makerDict_(emptyDict)
    { init(); };
    Image(Io io)
    : io_       (io)
    , makerDict_(emptyDict)
    { init(); };

    virtual    ~Image() {
        io_.close()      ;
    }

    void init(){
        start_         = 0 ;
        bigtiff_       = false;
        endian_        = keLittle;
        depth_         = 0;
        valid_         = false;
        serial_        = 0 ;
        shutterCount_  = 0 ;
        setMaker(kUnknown) ;
        lensSize_      = 0 ;
    }

    bool        valid()        { return false     ; }
    std::string path()         { return io_.path(); }
    endian_e    endian()       { return endian_   ; }
    Io&         io()           { return io_       ; }
    std::string format()       { return format_   ; }
    Visits&     visits()       { return visits_   ; }
    size_t      depth()        { return depth_    ; }
    bool        bigtiff()      { return bigtiff_  ; }
    virtual std::string uuidName(DataBuf& uuid,size_t offset=0) { return ""; }

    void visit(uint64_t address) { // never visit an address twice!
        if ( visits_.find(address) != visits_.end() ) {
            Error(kerAlreadyVisited,address);
        }
        visits_.insert(address);
    }

    virtual void accept(class Visitor& v)=0;

    maker_e     maker_;
    TagDict&    makerDict_;

    void setMaker(maker_e maker) {
        maker_ = maker;
        if ( makerDicts.find(maker) != makerDicts.end() ) {
            makerDict_ = *makerDicts[maker];
        }
    }

    void setMaker(DataBuf& buf)
    {
        maker_e result = kUnknown;
        // Don't do a look up because Makers have multiple names.  Use the short names in makers;
        for (Maker_i it = makers.begin(); it != makers.end(); it++) {
            std::string key  (it->first);
            std::string maker((const char*)buf.pData_,key.size());
            if ( isEqualsNoCase(maker,key) ) {
                result = it->second;
            }
        }
        setMaker(result);
    } // setMaker

    friend class ReportVisitor;
    friend class IFD          ;


protected:
    bool        valid_ ;
    Visits      visits_;
    uint64_t    start_;
    Io          io_;
    bool        good_;
    uint16_t    magic_;
    endian_e    endian_;
    bool        bigtiff_;
    size_t      depth_;
    std::string format_       ; // "TIFF", "JPEG" etc...
    std::string header_       ; // title for report

    // store    Nikon.LensData tag
    uint32_t    serial_       ; // required by nikon_decrypt
    uint32_t    shutterCount_ ; // required by nikon_decrypt
    uint64_t    lensAddress_  ;
    type_e      lensType_     ;
    uint32_t    lensCount_    ;
    uint64_t    lensOffset_   ;
    std::string lensOffsetS_  ;
    uint16_t    lensTag_      ;
    byte        lensData_[200];
    uint64_t    lensSize_     ;

    friend class ImageEndianSaver;
};

class ImageEndianSaver
{
public:
    ImageEndianSaver(Image& image,endian_e endian)
    : image_ (image)
    , endian_(endian)
    {}
    virtual ~ImageEndianSaver() { image_.endian_ = endian_ ; }
private:
    Image&   image_;
    endian_e endian_;
};

class IFD
{
public:
    IFD(Image& image,size_t start,bool next=true)
    : image_  (image)
    , start_  (start)
    , io_     (image.io())
    , next_   (next)
    {};

    void     accept        (Visitor& visitor,const TagDict& tagDict=tiffDict);
    void     visitMakerNote(Visitor& visitor,DataBuf& buf,uint64_t count,uint64_t offset);
    void     setIo         (Io& io) { io_ = io; }
    void     setStart      (uint64_t start) { start_ = start; }

    Visits&  visits()    { return image_.visits()  ; }
    maker_e  maker()     { return image_.maker_    ; }
    TagDict& makerDict() { return image_.makerDict_; }
    endian_e endian()    { return image_.endian()  ; }
    void     next(bool next)     { next_ = next ; }

    uint64_t get4or8(DataBuf& dir,uint64_t jump,uint64_t offset,endian_e endian)
    {
        bool     bigtiff = image_.bigtiff_ ;
        offset          *= bigtiff ? 8 : 4 ;
        return   bigtiff ? getLong8(dir,jump+offset,endian) : getLong (dir,jump+offset,endian);
    }

private:
    bool     next_   ;
    Image&   image_  ;
    size_t   start_  ;
    Io&      io_     ;
};

// Concrete Images with an accept() method which calls Vistor virtual functions (visitBegin/End, visitTag etc.)
class TiffImage : public Image
{
public:
    TiffImage(std::string path)
    : Image(path)
    , next_(true)
    {}
    TiffImage(Io& io,bool next=false,maker_e maker=kUnknown)
    : Image(io)
    , next_(next)
    { setMaker(maker);}

    bool valid()
    {
        if ( !valid_ ) {
            IoSave restore(io(),0);

            // read header
            DataBuf  header(16);
            io_.read(header);

            char c   = (char) header.pData_[0] ;
            char C   = (char) header.pData_[1] ;
            endian_  = c == 'M' ? keBig : keLittle;
            magic_   = getShort(header,2,endian_);
            bigtiff_ = magic_ == 43;
            start_   = bigtiff_ ? getLong8(header,8,endian_) : getLong (header,4,endian_);
            format_  = bigtiff_ ? "BIGTIFF"                  : "TIFF"                    ;
            header_  = " address |    tag                                  |      type |    count |    offset | value" ;

            uint16_t bytesize = bigtiff_ ? getShort(header,4,endian_) : 8;
            uint16_t version  = bigtiff_ ? getShort(header,6,endian_) : 0;
                                             // Pana     // Olym     R O // Olym     RS   // DCP
            valid_ =  (magic_==42||magic_==43||magic_==85||magic_==0x4f52||magic_==0x5352 ||magic_==0x4352) && (c == C) && (c=='I'||c=='M') && bytesize == 8 && version == 0;
            // Panasonic have augmented tiffDict with their keys
            if ( magic_ == 85 ) {
                setMaker(kPana);
                for ( TagDict::iterator it = panaDict.begin() ; it != panaDict.end() ; it++ ) {
                    if ( it->first != ktGroup ) {
                        tiffDict[it->first] = it->second;
                    }
                }
            }
        }
        return valid_ ;
    }
    void accept(class Visitor& visitor);
    void accept(Visitor& visitor,TagDict& tagDict);

    bool bigtiff()  { return bigtiff_ ; }
private:
    bool next_;
};

// 4. Create concrete "visitors"
class ReportVisitor: public Visitor
{
public:
    ReportVisitor(std::ostream& out, PSOption option)
    : Visitor(out,option)
    {
    }
    
    void visitBegin(Image& image,std::string msg="");
    void visitDirBegin(Image& image,uint64_t nEntries)
    {
        //size_t depth = image.depth();
        //out() << indent(depth) << stringFormat("+%d",nEntries) << std::endl;
    }
    void visitDirEnd(Image& image,uint64_t start)
    {
        // if ( start ) out() << std::endl;
    }

    void visitExif(Io&      io );

    bool printTag(std::string& name)
    {
        return name.find(".0x") == std::string::npos
               ||      option()  & kpsUnknown
               ;
    }

    void visitTag     ( Io&  io,Image& image, uint64_t  address
                      , uint16_t         tag, type_e       type
                      , uint32_t       count, uint64_t offset
                      , DataBuf&         buf, const TagDict& tagDict);
    void showError    (std::string message ) { out() << indent() << message << std::endl; }

    void visitEnd(Image& image)
    {
        if ( isBasicOrRecursive() ) {
            out() << indent() << "END: " << image.path() << std::endl;
        }
        indent_--;
        image.depth_--;
    }; // visitEnd

    // called by visitTag()
    void reportTag   (std::string& name,uint64_t address,endian_e endian,uint16_t tag,type_e type,uint32_t count,const DataBuf& buff,std::string offsetS);
    void reportFields(std::string& name,uint64_t address,endian_e endian,uint16_t tag,type_e type,uint32_t count,const DataBuf& buff,TagDict& makerDict);
};

void IFD::visitMakerNote(Visitor& visitor,DataBuf& buf,uint64_t count,uint64_t offset)
{
    bool bAdobe = buf.begins("Adobe");
    switch ( image_.maker_ ) {
        case kUnknown     : /* do nothing */ ; break;
        default           : /* do nothing */ ; break;

        case kNikon       : {
                // MakerNote is embedded tiff `II*_....` 10 bytes into the data!
                size_t        punt = buf.strequals("Nikon") ? 10 : 0 ;
                if ( bAdobe ) punt = 30              ;  // Adobe_MakN_.Q.II__..Nikon_..__II*_.___?_
                Io     io(io_,offset+punt,count-punt);
                TiffImage makerNote(io,image_.maker_);
                makerNote.accept(visitor,makerDict());
            } break;

        case kAgfa :
            if ( buf.strequals("ABC") ) {
                // MakerNote is an IFD `ABC_IIdL...`  6 bytes into the data!
                ImageEndianSaver save(image_,keLittle);
                IFD makerNote(image_,offset+6,false);
                makerNote.accept(visitor,makerDict());
            } break;

        case kApple:
            if ( buf.strequals("Apple iOS")) {
                // IFD `Apple iOS__.MM_._._.___.___._._.__..`  26 bytes into the data!
                ImageEndianSaver save(image_,keBig);
                IFD makerNote(image_,offset+26,false);
                makerNote.accept(visitor,makerDict());
            } break;

        case kMino : {
                ImageEndianSaver save(image_,keBig);  // Always bigEndian
                IFD makerNote(image_,offset,false);
                makerNote.accept(visitor,makerDict());
            } break;

        case kOlym:
            if ( buf.begins("OLYMPUS\0II")  ) { // "OLYMPUS\0II\0x3\0x0"E# or "OLYMP\0"
                Io     io(io_,offset,count);
                TiffImage makerNote(io,image_.maker_);
                makerNote.start_ =  12 ;
                makerNote.valid_ = true; // Valid without magic=42
                makerNote.accept(visitor,makerDict());
            } else {
                size_t punt = 8 ; // "OLYMPUS\0shortE#"
                IFD makerNote(image_,offset+punt,false);
                makerNote.accept(visitor,makerDict());
            } break;

        case kFuji: {
                Io     io(io_,offset,count);
                TiffImage makerNote(io,image_.maker_);
                makerNote.start_ = 12  ; // "FUJIFILM" 0x0c000000
                makerNote.valid_ = true; // Valid without magic=42
                makerNote.accept(visitor,makerDict());
            } break;

        case kPentax: {
                size_t punt = buf.begins("AOC") ? 6 : 10 ;
                ImageEndianSaver save(image_,keBig);
                IFD makerNote(image_,offset+punt,false);
                makerNote.accept(visitor,makerDict());
            } break;

        case kCanon: {
                IFD makerNote(image_,offset,true);
                makerNote.accept(visitor,makerDict());
            } break;

        case kSony: {
                size_t punt  = buf.strequals("SONY DSC ") ? 12 : 0; // Sony 12 byte punt
                IFD makerNote(image_,offset+punt,false);
                makerNote.accept(visitor,makerDict());
            } break;
    }
} // visitMakerNote

void IFD::accept(Visitor& visitor,const TagDict& tagDict/*=tiffDict*/)
{
    IoSave   save(io_,start_);
    bool     bigtiff = image_.bigtiff();
    endian_e endian  = image_.endian();

    if ( !image_.depth_ ) image_.visits().clear();
    visitor.visitBegin(image_);
    if ( image_.depth_ > 100 ) Error(kerCorruptedMetadata) ; // weird file

    // buffer
    DataBuf  entry(bigtiff ? 20 : 12);
    uint64_t start=start_;
    while  ( start ) {
        // Read top of directory
        io_.seek(start);
        io_.read(entry.pData_, bigtiff ? 8 : 2);
        uint64_t nEntries = bigtiff ? getLong8(entry,0,endian) : getShort(entry,0,endian);

        if ( nEntries > 500 ) {
            // don't Error (== panic) when the directory size seems impossible
            // Error(kerTiffDirectoryTooLarge,nEntries);
            // print error message and terminate the visit
            visitor.showError(stringFormat(" ** directory too large %d **",nEntries));
            visitor.visitEnd(image_);
            return ;
        }

        visitor.visitDirBegin(image_,nEntries);
        uint64_t a0 = start + (bigtiff?8:2) + nEntries * entry.size_; // address to read next

        // Run along the directory
        for ( uint64_t i = 0 ; i < nEntries ; i ++ ) {
            const uint64_t address = start + (bigtiff?8:2) + i* entry.size_ ;
            image_.visit(address); // never visit the same place twice!
            io_.seek(address);

            io_.read(entry);
            uint16_t tag    = getShort(entry,  0,endian);
            type_e   type   = getType (entry,  2,endian);
            uint32_t count  = (uint32_t) get4or8 (entry,4,0,endian);
            uint32_t offset = (uint32_t) get4or8 (entry,4,1,endian);

            if ( !typeValid(type,bigtiff) ) {
                Error(kerInvalidTypeValue,type);
            }

            uint64_t size   = typeSize(type) ;
            size_t   alloc  = size*count     ;
            DataBuf  buff(alloc);
            if ( alloc <= (bigtiff?8:4) ) {
                IoSave save(io_,address+2+2 + (bigtiff?8:4)); // step over tag/type/count
                io_.read(buff);
            } else {
                IoSave save(io_,offset);
                io_.read(buff);
            }
            if ( tagDict == tiffDict && tag == ktMake ) image_.setMaker(buff);
            visitor.visitTag(io_,image_,address,tag,type,count,offset,buff,tagDict);  // Tell the visitor

            if ( type == kttIfd  ) {
                for ( uint64_t i = 0 ; i < count ; i++ ) {
                    offset = buff.getLong(i*4,endian);
                    IFD(image_,offset,false).accept(visitor,ifdDict(image_.maker_,tag,makerDict()));
                }
            } else if ( type == kttIfd8  ) {
                for ( uint64_t i = 0 ; i < count ; i++ ) {
                    offset = (uint32_t) buff.getLong8(i*8,endian);
                    IFD(image_,offset,false).accept(visitor,ifdDict(image_.maker_,tag,makerDict()));
                }
            } else if ( tag == ktSubIFD  ) {
                for ( uint64_t i = 0 ; i < count ; i++ ) {
                    offset = (uint32_t) get4or8 (buff,0,i,endian);
                    IFD(image_,offset,false).accept(visitor,tagDict);
                }
            } else switch ( tag ) {
                case ktGps    : IFD(image_,offset,false).accept(visitor,gpsDict );break;
                case ktExif   : IFD(image_,offset,false).accept(visitor,exifDict);break;
                case ktIOP    : IFD(image_,offset,true ).accept(visitor,exifDict);break;
                case ktMNP    : /* Pentax and DNGPrivateData */
                case ktMN     : visitMakerNote(visitor,buff,count,offset)        ;break;
                default       : /* do nothing                                  */;break;
            }
        } // for i < nEntries

        start = 0; // !stop
        if ( next_ ) {
            io_.seek(a0);
            io_.read(entry.pData_, bigtiff?8:4);
            start = bigtiff?getLong8(entry,0,endian):getLong(entry,0,endian);
        }
        visitor.visitDirEnd(image_,start);
    } // while start != 0

    visitor.visitEnd(image_);
} // IFD::accept

void TiffImage::accept(class Visitor& visitor)
{
    accept(visitor,tiffDict);
}

void TiffImage::accept(Visitor& visitor,TagDict& tagDict)
{
    if ( valid() ) {
        IFD ifd(*this,start_,next_);
        ifd.accept(visitor,tagDict);
    } else {
        std::ostringstream os ; os << "expected " << format_ ;
        Error(kerInvalidFileFormat,io().path(), os.str());
    }
} // TiffImage::accept

void ReportVisitor::visitBegin(Image& image,std::string msg)
{
    image.depth_++;
    indent_++;
    if ( isBasicOrRecursive() ) {
        char c = image.endian() == keBig ? 'M' : 'I';
        out() << indent() << stringFormat("STRUCTURE OF %s FILE (%c%c): ",image.format().c_str(),c,c) <<  image.io().path() << std::endl;
        if ( msg.size() ) out() << indent() << msg << std::endl;
        if ( image.header_.size() ) {
            out() << indent() << image.header_ << std::endl;
        }
    }
}

void ReportVisitor::reportFields(std::string& name,uint64_t address,endian_e endian,uint16_t tag,type_e type,uint32_t count,const DataBuf& buff,TagDict& makerDict)
{
    if ( makerTags.find(name) == makerTags.end() ) return;

    for (Field field : makerTags[name]) {
        std::string n    = join(groupName(makerDict),field.name(),32);
        size_t      byte = field.start() + field.count() * typeSize(field.type());
        if ( byte <= buff.size_ ) {
            out() << indent()
                  << stringFormat("%8u | %#06x %-32s |%10s |%9u |%10s | "
                                 ,address+field.start(),tag,n.c_str(),typeName(field.type()),field.count(),"")
                  << chop(buff.toString(field.type(),field.count(),endian,field.start()),40)
                  << std::endl
            ;
        }
    }
}

void ReportVisitor::reportTag(std::string& name,uint64_t address,endian_e endian,uint16_t tag,type_e type,uint32_t count,const DataBuf& buff,std::string offsetS)
{
    if ( printTag(name) ) {
        out() << indent()
              << stringFormat("%8u | %#06x %-32s |%10s |%9u |%10s | "
                   ,address,tag,name.c_str(),::typeName(type),count,offsetS.c_str())
              << chop(buff.toString(type,count,endian),40)
              << std::endl;
    }
}

void ReportVisitor::visitTag
( Io&            io
, Image&         image
, uint64_t       address
, uint16_t       tag
, type_e         type
, uint32_t       count
, uint64_t       offset
, DataBuf&       buff
, const TagDict& tagDict
) {
    if ( !isBasicOrRecursive() ) return ;

    std::string offsetS ;
    if ( (uint64_t) (typeSize(type)*count) > (image.bigtiff_?8:4) ) {
        std::ostringstream os ;
        os  <<  offset;
        offsetS         = os.str();
    }

    std::string    name = tagName(tag,tagDict,32);
    std::string   value = buff.toString(type,count,image.endian_);

    if ( name == "Exif.Sony.FocalPosition" || name == "Exif.Sony.Tagx2010" ) {
        reportTag(name,address,image.endian_,tag,type,count,buff,offsetS);
		sonyCipher(buff.pData_,(uint32_t)buff.size_,true);
	}	

    if ( printTag(name) ) {
        reportTag(name,address,image.endian_,tag,type,count,buff,offsetS);
        if ( name != "Exif.Nikon.LensData" ) {
            reportFields(name,offset,image.endian(),tag,type,count,buff,image.makerDict_);
        }
    }

    // save nikon encrypted data until it can be decoded
    if ( name == "Exif.Nikon.SerialNumber" ) image.serial_       = atoi((char*) buff.pData_) ;
    if ( name == "Exif.Nikon.ShutterCount" ) image.shutterCount_ = buff.getLong(0,image.endian());
    // save lensData
    if ( name == "Exif.Nikon.LensData" && sizeof(image.lensData_) > buff.size_ ) {
        memcpy(image.lensData_,buff.pData_,buff.size_);
        image.lensSize_    = buff.size_;
        image.lensAddress_ = address ;
        image.lensType_    = type    ;
        image.lensCount_   = count   ;
        image.lensTag_     = tag     ;
        image.lensOffset_  = offset  ;
        image.lensOffsetS_ = offsetS ;
    }
    // restore and decode lensData
    if ( image.serial_ && image.shutterCount_ && image.lensSize_ ) {
        name = "Exif.Nikon.LensData";
        address = image.lensAddress_ ;
        tag     = image.lensTag_     ;
        type    = image.lensType_    ;
        count   = image.lensCount_   ;
        offset  = image.lensOffset_  ;
        offsetS = image.lensOffsetS_ ;

        nikon_decrypt(image.serial_,image.shutterCount_,tag,4,image.lensSize_,image.lensData_);
        DataBuf   lens(image.lensSize_);
        lens.copy(image.lensData_,image.lensSize_);

        reportTag   (name,address,image.endian(),tag,type,count,lens,offsetS);
        reportFields(name, offset,image.endian(),tag,type,count,lens,image.makerDict_);
        image.serial_ = 0; // don't do this again.
    }

} // visitTag

void ReportVisitor::visitExif(Io& io)
{
    if ( isRecursive() ) {
        // Beautiful.  io is a tiff file, call TiffImage::accept(visitor)
        TiffImage(io,true).accept(*this);
    }
}

static int hexToString(char buff[],int length)
{
    int  r     = 0   ; // resulting length
    int  t     = 0   ; // temporary
    bool first = true;
    bool valid[256];
    int  value[256];
    for ( int i =  0  ; i < 256 ; i++ ) valid[i] = false;
    for ( int i = '0' ; i <= '9' ; i++ ) {
        valid[i] = true;
        value[i] = i - '0';
    }
    for ( int i = 'a' ; i <= 'f' ; i++ ) {
        valid[i] = true;
        value[i] = 10 + i - 'a';
    }
    for ( int i = 'A' ; i <= 'F' ; i++ ) {
        valid[i] = true;
        value[i] = 10 + i - 'A';
    }

    for (int i = 0; i < length ; i++ )
    {
        char x = buff[i];
        if ( valid[x]  ) {
            if ( first ) {
                t     = value[x] << 4;
                first = false ;
            } else  {
                first  = true;
                buff[r++] = t + value[x];
            }
        }
    }
    return r;
}

std::unique_ptr<Image> ImageFactory(std::string path)
{
    TiffImage tiff(path); if ( tiff.valid() ) return std::unique_ptr<Image> (new TiffImage(path));
    return NULL;
}

void init(); // prototype

int main(int argc,const char* argv[])
{
    error_program = std::string(argv[0]);
    init();

    int rc  = 0;
    int arg = 1; // argument of interest
    if ( argc >= 2 ) {
        // Parse the visitor options
        PSOption option = kpsRecursive | kpsUnknown ;
        // create the visitor
        ReportVisitor visitor(std::cout,option);

        // step path arguments
        while ( arg < argc ) {
            // Open the image
            std::string             path  = argv[arg++];
            error_path                    = path;
            std::unique_ptr<Image> pImage = ImageFactory(path);
            if (   pImage ) pImage->accept(visitor);
            else            Error(kerUnknownFormat,path);
        }
    } else {
        std::cout << "usage: " << argv[0]  << " path+" << std::endl;
        rc = 1;
    }
    return rc;
}

// simply data.  Nothing interesting.
void init()
{
    if ( tiffDict.size() ) return; // don't do this twice!

    makers["Canon"      ]  = kCanon  ; makerDicts[kCanon ] = &canonDict;
    makers["NIKON"      ]  = kNikon  ; makerDicts[kNikon ] = &nikonDict;
    makers["SONY"       ]  = kSony   ; makerDicts[kSony  ] = &sonyDict;
    makers["AGFA"       ]  = kAgfa   ; makerDicts[kAgfa  ] = &agfaDict;
    makers["Apple"      ]  = kApple  ; makerDicts[kApple ] = &appleDict;
    makers["Panasonic"  ]  = kPana   ; makerDicts[kPana  ] = &panaDict;
    makers["Minolta"    ]  = kMino   ; makerDicts[kMino  ] = &minoDict;
    makers["OLYMPUS"    ]  = kOlym   ; makerDicts[kOlym  ] = &olymDict;
    makers["FUJIFILM"   ]  = kFuji   ; makerDicts[kFuji  ] = &fujiDict;
    makers["PENTAX"     ]  = kPentax ; makerDicts[kPentax] = &pentaxDict;
    // alias makers
    makers["RICOH"              ] = kPentax; 
    makers["SAMSUNG DIGITAL IMA"] = kPentax; 

    tiffDict  [ktGroup ] = "Image";
    tiffDict  [ ktExif ] = "ExifTag";
    tiffDict  [ ktGps  ] = "GPSTag";
    tiffDict  [ ktMake ] = "Make";
    tiffDict  [ ktIPTC ] = "IPTCNAA";
    tiffDict  [ ktXML  ] = "XMLPacket";
    tiffDict  [ ktICC  ] = "InterColorProfile";
    tiffDict  [ ktSubIFD]= "SubIFD";
    tiffDict  [ 0x00fe ] = "NewSubfileType";
    tiffDict  [ 0x00ff ] = "SubfileType";
    tiffDict  [ 0x0100 ] = "ImageWidth";
    tiffDict  [ 0x0101 ] = "ImageLength";
    tiffDict  [ 0x0102 ] = "BitsPerSample";
    tiffDict  [ 0x0103 ] = "Compression";
    tiffDict  [ 0x0106 ] = "PhotometricInterpretation";
    tiffDict  [ 0x010e ] = "ImageDescription";
    tiffDict  [ 0x0110 ] = "Model";
    tiffDict  [ 0x0111 ] = "StripOffsets";
    tiffDict  [ 0x0112 ] = "Orientation";
    tiffDict  [ 0x0115 ] = "SamplesPerPixel";
    tiffDict  [ 0x0116 ] = "RowsPerStrip";
    tiffDict  [ 0x0117 ] = "StripByteCounts";
    tiffDict  [ 0x011a ] = "XResolution";
    tiffDict  [ 0x011b ] = "YResolution";
    tiffDict  [ 0x011c ] = "PlanarConfiguration";
    tiffDict  [ 0x0128 ] = "ResolutionUnit";
    tiffDict  [ 0x0129 ] = "PageNumber";
    tiffDict  [ 0x0131 ] = "Software";
    tiffDict  [ 0x0132 ] = "DateTime";
    tiffDict  [ 0x013b ] = "Artist";
    tiffDict  [ 0x0201 ] = "JPEGInterchangeFormat";
    tiffDict  [ 0x0202 ] = "JPEGInterchangeLength";
    tiffDict  [ 0x0213 ] = "YCbCrPositioning";
    // DNG Tags
    dngDict   [ktGroup ] = "DNG";
    dngDict   [ 0xc614 ] = "UniqueCameraModel";
    dngDict   [ 0xc621 ] = "ColorMatrix1";
    dngDict   [ 0xc622 ] = "ColorMatrix2";
    dngDict   [ 0xc634 ] = "DNGPrivateData";
    dngDict   [ 0xc65a ] = "CalibrationIlluminant1";
    dngDict   [ 0xc65b ] = "CalibrationIlluminant2";
    dngDict   [ 0xc6f4 ] = "ProfileCalibrationSignature";
    dngDict   [ 0xc6f8 ] = "ProfileName";
    dngDict   [ 0xc6fc ] = "ProfileToneCurve";
    dngDict   [ 0xc6fd ] = "ProfileEmbedPolicy";
    dngDict   [ 0xc6fe ] = "ProfileCopyright";
    dngDict   [ 0xc714 ] = "ForwardMatrix1";
    dngDict   [ 0xc715 ] = "ForwardMatrix2";
    dngDict   [ 0xc725 ] = "ProfileLookTableDims";
    dngDict   [ 0xc726 ] = "ProfileLookTableData";
    dngDict   [ 0xc7a4 ] = "ProfileLookTableEncoding";
    dngDict   [ 0xc7a5 ] = "BaselineExposureOffset";
    dngDict   [ 0xc7a6 ] = "DefaultBlackRender";

    exifDict  [ktGroup ] = "Photo";
    exifDict  [ ktMN   ] = "MakerNote";
    exifDict  [ ktMNP  ] = "MakerNotePentax";
    exifDict  [ ktIOP  ] = "InteropTag";
    exifDict  [ 0x0001 ] = "InteropIndex";
    exifDict  [ 0x0002 ] = "InteropVersion";
    exifDict  [ 0x9286 ] = "UserComment";
    exifDict  [ 0x829a ] = "ExposureTime";
    exifDict  [ 0x829d ] = "FNumber";
    exifDict  [ 0x8822 ] = "ExposureProgram";
    exifDict  [ 0x8827 ] = "ISOSpeedRatings";
    exifDict  [ 0x8830 ] = "SensitivityType";
    exifDict  [ 0x9000 ] = "ExifVersion";
    exifDict  [ 0x9003 ] = "DateTimeOriginal";
    exifDict  [ 0x9004 ] = "DateTimeDigitized";
    exifDict  [ 0x9101 ] = "ComponentsConfiguration";
    exifDict  [ 0x9102 ] = "CompressedBitsPerPixel";
    exifDict  [ 0x9204 ] = "ExposureBiasValue";
    exifDict  [ 0x9205 ] = "MaxApertureValue";
    exifDict  [ 0x9207 ] = "MeteringMode";
    exifDict  [ 0x9208 ] = "LightSource";
    exifDict  [ 0x9209 ] = "Flash";
    exifDict  [ 0x920a ] = "FocalLength";
    exifDict  [ 0xa000 ] = "FlashpixVersion";
    exifDict  [ 0xa001 ] = "ColorSpace";
    exifDict  [ 0xa002 ] = "PixelXDimension";
    exifDict  [ 0xa003 ] = "PixelYDimension";
    exifDict  [ 0xa300 ] = "FileSource";
    exifDict  [ 0xa301 ] = "SceneType";
    exifDict  [ 0xa401 ] = "CustomRendered";
    exifDict  [ 0xa402 ] = "ExposureMode";
    exifDict  [ 0xa403 ] = "WhiteBalance";
    exifDict  [ 0xa406 ] = "SceneCaptureType";
    exifDict  [ 0xa408 ] = "Contrast";
    exifDict  [ 0xa409 ] = "Saturation";
    exifDict  [ 0xa40a ] = "Sharpness";
    exifDict  [ 0xc4a5 ] = "PrintImageMatching";

    gpsDict   [ktGroup ] = "GPSInfo";
    gpsDict   [ 0x0000 ] = "GPSVersionID";
    gpsDict   [ 0x0001 ] = "GPSLatitudeRef";
    gpsDict   [ 0x0002 ] = "GPSLatitude";
    gpsDict   [ 0x0003 ] = "GPSLongitudeRef";
    gpsDict   [ 0x0004 ] = "GPSLongitude";
    gpsDict   [ 0x0005 ] = "GPSAltitudeRef";
    gpsDict   [ 0x0006 ] = "GPSAltitude";
    gpsDict   [ 0x0007 ] = "GPSTimeStamp";
    gpsDict   [ 0x0008 ] = "GPSSatellites";
    gpsDict   [ 0x0012 ] = "GPSMapDatum";
    gpsDict   [ 0x001d ] = "GPSDateStamp";

    nikonDict [ktGroup ] = "Nikon";
    nikonDict [ 0x0001 ] = "Version";
    nikonDict [ 0x0002 ] = "ISOSpeed";
    nikonDict [ 0x0004 ] = "Quality";
    nikonDict [ 0x0005 ] = "WhiteBalance";
    nikonDict [ 0x0007 ] = "Focus";
    nikonDict [ 0x0008 ] = "FlashSetting";
    nikonDict [ 0x0009 ] = "FlashDevice";
    nikonDict [ 0x000b ] = "WhiteBalanceBias";
    nikonDict [ 0x000c ] = "WB_RBLevels";
    nikonDict [ 0x000d ] = "ProgramShift";
    nikonDict [ 0x000e ] = "ExposureDiff";
    nikonDict [ 0x0012 ] = "FlashComp";
    nikonDict [ 0x0013 ] = "ISOSettings";
    nikonDict [ 0x0016 ] = "ImageBoundary";
    nikonDict [ 0x0017 ] = "FlashExposureComp";
    nikonDict [ 0x0018 ] = "FlashBracketComp";
    nikonDict [ 0x0019 ] = "ExposureBracketComp";
    nikonDict [ 0x001b ] = "CropHiSpeed";
    nikonDict [ 0x001c ] = "ExposureTuning";
    nikonDict [ 0x001d ] = "SerialNumber";
    nikonDict [ 0x001e ] = "ColorSpace";
    nikonDict [ 0x0023 ] = "PictureControl";
    nikonDict [ 0x0098 ] = "LensData";
    nikonDict [ 0x00a7 ] = "ShutterCount";

    // Fields
    makerTags["Exif.Nikon.PictureControl"].push_back(Field("PcVersion"         ,kttAscii , 0, 4));
    makerTags["Exif.Nikon.PictureControl"].push_back(Field("PcName"            ,kttAscii , 4,20));
    makerTags["Exif.Nikon.PictureControl"].push_back(Field("PcBase"            ,kttAscii ,24,20));
    makerTags["Exif.Nikon.PictureControl"].push_back(Field("PcAdjust"          ,kttUByte, 48, 1));
    makerTags["Exif.Nikon.PictureControl"].push_back(Field("PcQuickAdjust"     ,kttUByte, 49, 1));
    makerTags["Exif.Nikon.PictureControl"].push_back(Field("PcSharpness"       ,kttUByte, 50, 1));
    makerTags["Exif.Nikon.PictureControl"].push_back(Field("PcContrast"        ,kttUByte, 51, 1));
    makerTags["Exif.Nikon.PictureControl"].push_back(Field("PcBrightness"      ,kttUByte, 52, 1));
    makerTags["Exif.Nikon.PictureControl"].push_back(Field("PcSaturation"      ,kttUByte, 53, 1));
    makerTags["Exif.Nikon.PictureControl"].push_back(Field("PcHueAdjustment"   ,kttUByte, 54, 1));
    makerTags["Exif.Nikon.PictureControl"].push_back(Field("PcFilterEffect"    ,kttUByte, 55, 1));
    makerTags["Exif.Nikon.PictureControl"].push_back(Field("PcFilterEffect"    ,kttUByte, 56, 1));
    makerTags["Exif.Nikon.PictureControl"].push_back(Field("PcToningSaturation",kttUByte, 57, 1));

    // https://exiftool.org/TagNames/Nikon.html#LensData00 "Nikon LensData0800 Tags"
    makerTags["Exif.Nikon.LensData"      ].push_back(Field("LdVersion"         ,kttAscii , 0, 4));
    makerTags["Exif.Nikon.LensData"      ].push_back(Field("LdFocusDistance"   ,kttUByte , 9, 1));
    makerTags["Exif.Nikon.LensData"      ].push_back(Field("LdLensIDNumber"    ,kttUByte ,13, 1));
    makerTags["Exif.Nikon.LensData"      ].push_back(Field("LdLensID"          ,kttUShort,48, 1));

    canonDict [ktGroup ] = "Canon";
    canonDict [ 0x0001 ] = "CameraSettings";
    canonDict [ 0x0002 ] = "Selftimer";
    canonDict [ 0x0003 ] = "Quality";
    canonDict [ 0x0004 ] = "FlashMode";
    canonDict [ 0x0005 ] = "DriveMode";
    canonDict [ 0x0007 ] = "FocusMode";
    canonDict [ 0x000a ] = "ImageSize";
    canonDict [ 0x000b ] = "EasyMode";
    canonDict [ 0x000c ] = "DigitalZoom";
    canonDict [ 0x000d ] = "Contrast";
    canonDict [ 0x000e ] = "Saturation";
    canonDict [ 0x000f ] = "Sharpness";
    canonDict [ 0x0010 ] = "ISOSpeed";
    canonDict [ 0x0011 ] = "MeteringMode";
    canonDict [ 0x0012 ] = "FocusType";

    // https://exiftool.org/TagNames/Canon.html
    makerTags["Exif.Canon.CameraSettings"].push_back(Field("CsMacro"           ,kttUShort, 2 *1, 1));
    makerTags["Exif.Canon.CameraSettings"].push_back(Field("CsTimer"           ,kttUShort, 2 *2, 1));
    makerTags["Exif.Canon.CameraSettings"].push_back(Field("CsQuality"         ,kttUShort, 2 *3, 1));
    makerTags["Exif.Canon.CameraSettings"].push_back(Field("CsEasyMode"        ,kttUShort, 2*11, 1));
    makerTags["Exif.Canon.CameraSettings"].push_back(Field("CsSpotMeteringMode",kttUShort, 2*39, 1));

    sonyDict  [ktGroup ] = "Sony";
    sonyDict  [ 0x0001 ] = "Offset";
    sonyDict  [ 0x0002 ] = "ByteOrder";
    sonyDict  [ 0xb020 ] = "ColorReproduction";
    sonyDict  [ 0xb040 ] = "Macro";
    sonyDict  [ 0xb041 ] = "ExposureMode";
    sonyDict  [ 0xb042 ] = "FocusMode";
    sonyDict  [ 0xb043 ] = "AFMode";
    sonyDict  [ 0xb044 ] = "AFIlluminator";
    sonyDict  [ 0xb047 ] = "JPEGQuality";
    sonyDict  [ 0xb048 ] = "FlashLevel";
    sonyDict  [ 0xb049 ] = "ReleaseMode";
    sonyDict  [ 0xb04a ] = "SequenceNumber";
    sonyDict  [ 0xb04b ] = "AntiBlur";
    sonyDict  [ 0xb04e ] = "LongExposureNoiseReduction";
    sonyDict  [ 0x9402 ] = "FocalPosition";
    sonyDict  [ 0x2010 ] = "Tagx2010";

    // Fields
    makerTags["Exif.Sony.FocalPosition"].push_back(Field("FpAmbientTemperature" ,kttByte  , 0x04, 1));
    makerTags["Exif.Sony.FocalPosition"].push_back(Field("FpFocusMode"          ,kttUByte , 0x16, 1));
    makerTags["Exif.Sony.FocalPosition"].push_back(Field("FpAreaMode"           ,kttUByte , 0x17, 1));
    makerTags["Exif.Sony.FocalPosition"].push_back(Field("FpFocusPosition"      ,kttUByte , 0x2d, 1));

    makerTags["Exif.Sony.Tagx2010"     ].push_back(Field("WB_RGBLevels"         ,kttUShort, 4532, 3)); // https://exiftool.org/TagNames/Sony.html#Tag2010e

    agfaDict  [ktGroup ] = "Agfa";
    agfaDict  [ 0x0001 ] = "One";
    agfaDict  [ 0x0002 ] = "Size";
    agfaDict  [ 0x0003 ] = "Three";
    agfaDict  [ 0x0004 ] = "Four";
    agfaDict  [ 0x0005 ] = "Thumbnail";

    appleDict [ktGroup ] = "Apple";
    appleDict [ 0x0001 ] = "One";
    appleDict [ 0x0002 ] = "Two";
    appleDict [ 0x0003 ] = "Three";
    appleDict [ 0x0004 ] = "Four";
    appleDict [ 0x0005 ] = "Five";
    appleDict [ 0x0006 ] = "Six";
    appleDict [ 0x0007 ] = "Seven";
    appleDict [ 0x0008 ] = "Eight";
    appleDict [ 0x0009 ] = "Nine";
    appleDict [ 0x000a ] = "Ten";
    appleDict [ 0x000b ] = "Eleven";
    appleDict [ 0x000c ] = "Twelve";
    appleDict [ 0x000d ] = "Thirteen";

    panaDict  [ktGroup ] = "Panasonic";
    panaDict  [ 0x0001 ] = "Version";
    panaDict  [ 0x0002 ] = "SensorWidth";
    panaDict  [ 0x0003 ] = "SensorHeight";
    panaDict  [ 0x0004 ] = "SensorTopBorder";
    panaDict  [ 0x0005 ] = "SensorLeftBorder";
    panaDict  [ 0x0006 ] = "ImageHeight";
    panaDict  [ 0x0007 ] = "ImageWidth";
    panaDict  [ 0x0011 ] = "RedBalance";
    panaDict  [ 0x0012 ] = "BlueBalance";
    panaDict  [ 0x0017 ] = "ISOSpeed";
    panaDict  [ 0x0024 ] = "WBRedLevel";
    panaDict  [ 0x0025 ] = "WBGreenLevel";
    panaDict  [ 0x0026 ] = "WBBlueLevel";
    panaDict  [ 0x002e ] = "PreviewImage";

    minoDict  [ktGroup ] = "Minolta";
    minoDict  [ 0x0000 ] = "BlockDescriptor";
    minoDict  [ 0x0001 ] = "CameraSettings";
    minoDict  [ 0x0010 ] = "AutoFocus";
    minoDict  [ 0x0040 ] = "ImageSize";
    minoDict  [ 0x0081 ] = "Thumbnail";

    fujiDict  [ktGroup ] = "Fuji";
    fujiDict  [ 0x0000 ] = "Version";
    fujiDict  [ 0x0010 ] = "SerialNumber";
    fujiDict  [ 0x1000 ] = "Quality";
    fujiDict  [ 0x1001 ] = "Sharpness";
    fujiDict  [ 0x1002 ] = "WhiteBalance";
    fujiDict  [ 0xf000 ] = "FujiIFD";
    fujiDict  [ 0xf001 ] = "RawFullWidth";
    fujiDict  [ 0xf002 ] = "RawFullHeight";
    fujiDict  [ 0xf003 ] = "BitPerSample";
    fujiDict  [ 0xf007 ] = "StripOffsets";
    fujiDict  [ 0xf008 ] = "StripByteCounts";
    fujiDict  [ 0xf00a ] = "BlackLevel";
    fujiDict  [ 0xf00b ] = "GeometricDistortionParams";
    fujiDict  [ 0xf00c ] = "WB_GRBLevelsStandard";
    fujiDict  [ 0xf00d ] = "WB_GRBLevelsAuto";
    fujiDict  [ 0xf00e ] = "WB_GRBLevels";
    fujiDict  [ 0xf00f ] = "ChromaticAberrationParams";
    fujiDict  [ 0xf010 ] = "VignettingParams";

    olymDict  [ktGroup ] = "Olympus";
    olymDict  [ 0x0100 ] = "ThumbnailImage";
    olymDict  [ 0x0104 ] = "BodyFirmwareVersion";
    olymDict  [ 0x0200 ] = "SpecialMode";
    olymDict  [ 0x0201 ] = "Quality";
    olymDict  [ 0x0202 ] = "Macro";
    olymDict  [ 0x0203 ] = "BWMode";
    olymDict  [ 0x0204 ] = "DigitalZoom";
    // Olympus IFDs
    olymDict  [ 0x2010 ] = "Equipment";  // see ifdDict()
    olymDict  [ 0x2020 ] = "CameraSettings";
    olymDict  [ 0x2030 ] = "RawDevelopment";
    olymDict  [ 0x2031 ] = "RawDevelopment2";
    olymDict  [ 0x2040 ] = "ImageProcessing";
    olymDict  [ 0x2050 ] = "FocusInfo";
    olymDict  [ 0x3000 ] = "RawInfo";

    olymCSDict[ktGroup ] = "OlympusCS";
    olymCSDict[ 0x0000 ] = "Version";
    olymCSDict[ 0x0100 ] = "PreviewImageValid";
    olymCSDict[ 0x0101 ] = "PreviewImageStart";
    olymCSDict[ 0x0102 ] = "PreviewImageLength";
    olymCSDict[ 0x0200 ] = "ExposureMode";

    olymEQDict[ktGroup ] = "OlympusEQ";
    olymEQDict[ 0x0000 ] = "Version";
    olymEQDict[ 0x0100 ] = "CameraType";
    olymEQDict[ 0x0101 ] = "SerialNumber";
    olymEQDict[ 0x0102 ] = "InternalSerialNumber";

    olymRDDict[ktGroup ] = "OlymRawDev";
    olymRDDict[ 0x0000 ] = "Version";
    olymRDDict[ 0x0100 ] = "ExposureBiasValue";
    olymRDDict[ 0x0101 ] = "WhiteBalanceValue";
    olymRDDict[ 0x0102 ] = "WBFineAdjustment";
    olymRDDict[ 0x0103 ] = "GrayPoint";

    olymR2Dict[ktGroup ] = "OlymRawDev2";
    olymR2Dict[ 0x0000 ] = "Version";
    olymR2Dict[ 0x0100 ] = "ExposureBiasValue";
    olymR2Dict[ 0x0101 ] = "WhiteBalance";
    olymR2Dict[ 0x0102 ] = "WhiteBalanceValue";
    olymR2Dict[ 0x0103 ] = "WBFineAdjustment";
    olymR2Dict[ 0x0104 ] = "GrayPoint";

    olymIPDict[ktGroup ] = "OlymImgProc";
    olymIPDict[ 0x0000 ] = "Version";
    olymIPDict[ 0x0100 ] = "WB_RBLevels";

    olymFIDict[ktGroup ] = "OlymFocusInfo";
    olymFIDict[ 0x0000 ] = "Version";
    olymFIDict[ 0x0209 ] = "AutoFocus";
    olymFIDict[ 0x0210 ] = "SceneDetect";
    olymFIDict[ 0x0211 ] = "SceneArea";

    olymRoDict[ktGroup ] = "OlymRawInfo";
    olymRoDict[ 0x0000 ] = "Version";
    olymRoDict[ 0x0100 ] = "WB_RBLevelsUsed";
    olymRoDict[ 0x0110 ] = "WB_RBLevelsAuto";

    pentaxDict[ktGroup ] = "Pentax";
    pentaxDict[ 0x0000 ] = "Version";
    pentaxDict[ 0x0001 ] = "Mode";
    pentaxDict[ 0x0002 ] = "PreviewResolution";
    pentaxDict[ 0x0003 ] = "PreviewLength";
    pentaxDict[ 0x0004 ] = "PreviewOffset";
    pentaxDict[ 0x0005 ] = "ModelID";
    pentaxDict[ 0x0006 ] = "Date";
    pentaxDict[ 0x0007 ] = "Time";
    pentaxDict[ 0x0008 ] = "Quality";
    pentaxDict[ 0x0009 ] = "Size";
    pentaxDict[ 0x000e ] = "Flash";
    pentaxDict[ 0x000d ] = "Focus";
    pentaxDict[ 0x003f ] = "LensRec";
}

// That's all Folks!
////
