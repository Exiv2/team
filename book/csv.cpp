// http://www.zedwood.com/article/cpp-csv-parser
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
 
std::vector<std::string> read(std::istream &in, char delimiter)
{
    std::stringstream ss;
    bool inquotes = false;
    bool bEnd     = false;
    char Q = '"'  ; // quote character
    char L = '\n' ; // line-feed
    char C = '\r' ; // carriage-return

    std::vector<std::string> row;
    
    while(in.good() && !bEnd) {
        char c = in.get();
        if (!inquotes && c==Q) { 
            inquotes=true;
        } else if (inquotes && c==Q) { 
            if ( in.peek() == Q) { //2 consecutive quotes resolve to 1
                ss << (char)in.get();
            } else { //endquotechar
                inquotes=false;
            }
        } else if (!inquotes && c==delimiter) { //end of field
            row.push_back( ss.str() );
            ss.str("");
        } else if (!inquotes && (c==C || c==L) ) {
            if(in.peek()==L) { in.get(); }
            row.push_back( ss.str() );
            bEnd = true;
        } else {
          ss << c;
        }
    }
    return row;
}
 
int main(int argc, char *argv[])
{
    if ( argc != 2 ) {
        std::cerr << "usage: " << argv[0] << " { path | - }" << std::endl;
        return 1;
    }
    
    // open file and connect to std::cin
    std::string   path(argv[1]);
    std::ifstream file(path);     
    if ( path != "-" ) {
        if ( file.is_open() ) {
            std::cin.rdbuf(file.rdbuf());
        } else if ( argc > 1 ) { 
            std::cerr << "file did not open: " << path << std::endl;
            return 2;
        }
    }
    
    // parse input line by line
    while( std::cin.good() )
    {
        std::vector<std::string> row = read(std::cin , ',');
        for(int i=0, leng=row.size(); i<leng; i++)
            std::cout << "[" << row[i] << "]" << "\t";
        std::cout << std::endl;
    }
    
    file.close();
    return 0;
}
 
