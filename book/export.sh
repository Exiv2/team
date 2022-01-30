#!/usr/bin/env bash
sed -i .bak s'#<title>IMaEA</title>#<title>Image Metadata and Exiv2 Architecture</title>#' IMaEA.html
rm  -f *.bak
for i in $(find . -type f -depth 1); do cp -v $i ~/clanmills/exiv2/book ; done
cd ~/clanmills
mv exiv2/book/IMaEA.html exiv2/book/index.html
./syncup exiv2
ssh clanmil1@clanmills.com 'chmod 644 public_html/exiv2/book/index.html'

# That's all Folks!
##
