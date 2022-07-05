//
// Created by Eugenio Moro on 24/06/22.
//

#ifndef NANOPB_CMAKE_SIMPLE_E2PRTBF_COMMON_H
#define NANOPB_CMAKE_SIMPLE_E2PRTBF_COMMON_H

#include "pb.h"

pb_ostream_t pb_ostream_from_socket(int fd);
pb_istream_t pb_istream_from_socket(int fd);


#endif //NANOPB_CMAKE_SIMPLE_E2PRTBF_COMMON_H

