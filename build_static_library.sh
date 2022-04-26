SOURCE_DIR="./src/"
MAKE_DIR="./compilation/"

# Compile source
cd $MAKE_DIR
make -B
cd ..

# Build libray
OBJECT_FILES=$(find $SOURCE_DIR -name \*.o)
ar -rc pspp.a $OBJECT_FILES
