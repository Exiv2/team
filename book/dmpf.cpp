// g++ --std=c++11 dmpf.cpp
#include <stdio.h>
#include <map>
#include <string.h>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

std::vector      <std::string> paths;
std::string                    terminal("-");
std::map<std::string,uint32_t> options;

static enum error_e
{   errorOK = 0
,   errorSyntax
,   errorProcessing
}   error = errorOK ;

uint8_t print(uint8_t c) { return c >= 32 && c < 127 ? c : c==0 ? '_' : '.' ; }

void printOptions(error_e e)
{
    if ( options["verbose"] || e == errorSyntax ) {
        size_t count=0;
        std::cout << "options: ";
        for ( auto option : options ) {
            std::cout << (count++?" ":"") << option.first << "=" << option.second << "" ;
        }
    }
    std::cout << std::endl;
    error = e;
}

void syntax(int argc, char* argv[],error_e e)
{
    std::cout << "usage: " << argv[0] << " [-]+[key=value]+ path+" << std::endl;
    printOptions(errorSyntax) ;
}

bool split(const char* arg,std::string& key,uint32_t& value)
{
    while ( *arg == '-' ) arg++;
    const char* chop = std::strchr(arg,'=');
    if ( chop ) {
        key   = std::string(arg,chop-arg);
        value = atoi(chop+1);
    }
    return chop != NULL;
}

// endian and byte swappers
bool isPlatformBigEndian()
{
    union { uint32_t i; char c[4]; } e = { 0x01000000 };
    return e.c[0]?true:false;
}

uint32_t platformEndian() { return isPlatformBigEndian() ? 1 : 0; }
void swap(void* from,void* to,size_t n)
{
    uint8_t*     v = reinterpret_cast<uint8_t *>(from);
    uint8_t*  swap = reinterpret_cast<uint8_t *>(to  );
    for (size_t i = 0; i < n; i++) {
        swap[i] = v[n - i - 1];
    }
}
uint64_t swap(uint64_t* value,bool bSwap)
{
    uint64_t result = *value ;
    if ( bSwap ) swap(value,&result,sizeof result);
    return result;
}
uint32_t swap(uint32_t* value,bool bSwap)
{
    uint32_t result = *value ;
    if ( bSwap ) swap(value,&result,sizeof result);
    return result;
}
uint16_t swap(uint16_t* value,bool bSwap)
{
    uint16_t result = *value ;
    if ( bSwap ) swap(value,&result,sizeof result);
    return result;
}

std::vector<std::string> splitter (const std::string &s, char delim)
{
    std::vector<std::string> result;
    std::stringstream        ss (s);
    std::string              item;

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}

bool file(const char* arg,std::string& stub,uint32_t& skip,uint32_t& count)
{
    std::string path(arg);
    if ( path == terminal ) { stub = terminal ; return true ; }

    // parse path/to/file[:number+length]+
    std::vector<std::string> paths = ::splitter(path,':');
    for ( size_t i = 1 ; i < paths.size() ; i++ ) {
        std::vector<std::string> numbers;
        // path:start
        numbers   = splitter(paths[i],'+');
        if ( numbers.size() ) {
            skip     += ::atoi(numbers[0].c_str());
            // path:start->count
            numbers   = splitter(numbers[0],'>');
            if ( !count && numbers.size() > 1 ) {
                count = ::atoi(numbers[1].c_str());
            }
        }
    }
    FILE*  f      = ::fopen(paths[0].c_str(),"rb");
    bool   result = f != NULL ;
    if     (f) fclose(f);
    stub=paths[0];
    return result;
} //file

int main(int argc, char* argv[])
{
    options["bs"     ] =  1;
    options["width"  ] =  0;
    options["count"  ] =  0;
    options["endian" ] =  isPlatformBigEndian();
    options["hex"    ] =  1;
    options["skip"   ] =  0;
    options["verbose"] =  0;
    options["dryrun" ] =  0;
    options["start"  ] =  0; // set by file[:start->length]+

    // parse arguments
    if( (argc < 2)                                         // ./dmpf
    ||  (argc==2 && std::string(argv[1]).find("-h") != -1) // ./dmpf --help or ./dmpf -h => help
    ){
        syntax(argc,argv,errorSyntax) ;
    } else for ( int i = 1 ; i < argc ; i++ ) {
        const char* arg = argv[i];
        std::string key;
        std::string stub;
        uint32_t    value ;
        bool        bClaimed = false;
        if ( split(argv[i],key,value) ) {
            if ( options.find(key) != options.end() ) {
                bClaimed = true ;
                if ( key == "bs" || key == "hex" || key == "verbose" || key == "endian" || key == "dryrun" ) {
                    options[key] =value; // boolean keys
                } else {
                    options[key]+=value; // accumulative keys
                }
            }
        } else if ( file(arg,stub,options["start"],options["count"]) ) {
            paths.push_back(stub);
            bClaimed = true;
        }
        if ( !bClaimed ) {
            std::cerr << "argument not understood: " << arg << std::endl;
            error = errorProcessing;
        }
    }

    // report arguments
    if ( options["verbose"] || options["dryrun"]  ) printOptions(error) ;
    if ( options["dryrun"] ) exit(0);

    // process
    if ( !error  ) for ( auto path : paths ) {
        FILE* f = NULL  ;
        size_t  size   = 1;
        size_t  skip   = options["skip" ];
        size_t  count  = options["count"];
        size_t  width  = options["width"];
        size_t  start  = options["start"];

        if ( path != terminal ) {
            f     = fopen(path.c_str(),"rb");
            fseek(f,0,SEEK_END);
            size  = ftell(f);
        } else {
            f      = stdin   ;
            size   = 256*1024;
        }
        if ( !count ) count = size - skip-start;
        if ( !f || (skip+count+start) > size ) {
            std::cerr << path << " insufficient data" << std::endl;
            error = errorProcessing;
        }

        char    line[1000]  ;
        char    buff[64]    ;
        size_t  reads  = 0 ; // count the reads
        size_t  nRead  = 0 ; // bytes actually read
        size_t  remain = count ; // how many bytes still to read
        if ( width == 0 ) width = 32 ;
        if ( width > sizeof buff ) width = sizeof(buff);
        fseek(f,(long)skip+start,SEEK_SET);

        if ( !error ) while ( remain && (nRead = fread(buff,1,remain>width?width:remain,f)) > 0 ) {
            // line number
            int l = sprintf(line,"%#8lx %8ld: ",(unsigned long)(skip+reads*width), (unsigned long)(skip+reads*width) ) ;

            // ascii print
            for ( int i = 0 ; i < nRead ; i++ ) {
                l += sprintf(line+l,"%c", print(buff[i])) ;
            }

            // blank pad the ascii
            size_t  n   = nRead ;
            while ( n++ <  width ) {
                l += sprintf(line+l," ") ;
            }
            l += sprintf(line+l,"  -> ") ;

            size_t   bs = options["bs"];
            switch ( bs ) {
            case 8 :
                for ( size_t i = 0 ; i < nRead; i += bs ) {
                    uint64_t* p = (uint64_t*) &buff[i] ;
                    uint64_t  v = swap(p, options["endian"]!=platformEndian() );
                    l += options["hex"] ? sprintf(line+l," %16llx" ,(long long int)v )
                                        : sprintf(line+l," %20lld" ,(long long int)v )
                    ;
                }
            break;
            case 4 :
                for ( size_t i = 0 ; i < nRead ; i += bs ) {
                    uint32_t* p = (uint32_t*) &buff[i] ;
                    uint32_t  v = swap(p,  options["endian"]!=platformEndian());
                    l += options["hex"] ? sprintf(line+l,"  %8x" ,v )
                                        : sprintf(line+l," %10d" ,v )
                    ;
                }
            break;
            case 2:
                for ( size_t i = 0 ; i < nRead ; i += bs ) {
                    uint16_t* p = (uint16_t*) &buff[i] ;
                    uint16_t  v = swap(p,  options["endian"]!=platformEndian());
                    l += options["hex"] ? sprintf(line+l," %4x" ,v )
                                        : sprintf(line+l," %5d" ,v )
                    ;
                }
            break;
            default:
                for ( int i = 0 ; i < nRead ; i++ ) { // bs == 1
                    uint8_t v = buff[i];
                    l += options["hex"] ? sprintf(line+l," %02x" ,v )
                                    : sprintf(line+l," %3d" ,v )
                    ;
                }
            }

            line[l] = 0 ;
            std::cout << line << std::endl;
            reads++;
            remain -= nRead;
            if ( path == terminal ) size += nRead;
        } // while remains && nRead

        if ( f != stdin ) {
            fclose(f);
        }
        f = NULL;
    }

    return error ;
} // main
