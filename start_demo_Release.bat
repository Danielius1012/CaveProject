SET opensg_BIN=C:\libraries\install\vs11_32\opensg\bin\rel
SET glut_bin=C:\libraries\install\vs11_32\freeglut\bin
SET invrs_bin=C:\libraries\install\vs11_32\invrs\bin
SET tiff_bin=C:\libraries\install\vs11_32\libtiff\bin
SET png_bin=C:\libraries\install\vs11_32\libpng\bin
SET zlib_bin=C:\libraries\install\vs11_32\zlib\bin
SET jpeg_bin=C:\libraries\install\vs11_32\libjpeg-turbo\bin
SET PATH=%opensg_BIN%;%glut_bin%;%invrs_bin%;%tiff_bin%;%png_bin%;%zlib_bin%;%jpeg_bin%;%PATH%; 
.\exe\Release\MyProject.exe -f config\mono.csm