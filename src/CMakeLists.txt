
# Create the "compress" executable target
add_executable(Compress main_compress.cpp )
target_include_directories(Compress PUBLIC ${TERSE_INCLUDE_DIR})

# Create the "decompress" executable target
add_executable(Decompress main_decompress.cpp )
target_include_directories(Decompress PUBLIC ${TERSE_INCLUDE_DIR})


