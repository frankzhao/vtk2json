//
//  vtk2json.h
//  vtk2json
//
//  VTK to JSON conversion
//
//  Created by Frank Zhao on 29/04/2014.
//  Copyright (c) 2014 Frank Zhao. All rights reserved.
//

#ifndef vtk2json_vtk2json_h
#define vtk2json_vtk2json_h

/* VTK struct */
typedef struct t_vtk_ *p_vtk;

/* VTK format mode */
typedef enum e_vtk_mode_ {
    VTK_ASCII,
    VTK_BINARY,
    NONE
} e_vtk_mode;

/* VTK data type */
typedef enum e_vtk_data_type_ {
    VTK_FLOAT,
    VTK_DOUBLE
} e_vtk_data_type;

#endif
