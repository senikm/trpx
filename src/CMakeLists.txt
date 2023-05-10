
# Create the "compress" executable target
#add_executable(terse terse.cpp )
#target_include_directories(terse PUBLIC ${TERSE_INCLUDE_DIR})

# Create the "compress" executable target
add_executable(terse terse.cpp )
target_include_directories(terse PUBLIC ${TERSE_INCLUDE_DIR})

# Create the "compress" executable target
add_executable(prolix prolix.cpp )
target_include_directories(prolix PUBLIC ${TERSE_INCLUDE_DIR})




