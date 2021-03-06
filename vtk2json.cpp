//
//  main.cpp
//  vtk2json
//  VTK to JSON conversion
//
//  Created by Frank Zhao on 29/04/2014.
//  frank.zhao@anu.edu.au
//  Copyright (c) 2014 Frank Zhao. All rights reserved.
//
// TODO: I/O error handling

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vtk2json.h"

using namespace std;

bool debug = false;

typedef struct t_vtk_ {
    char* version;
    char* comment;
    char dataset[256];
    long npoints;
    long npolygons;
    long polysize;
    e_vtk_mode io_mode;
    e_vtk_data_type data_mode;
} t_vtk;

static ifstream fs; // VTK filestream
static ofstream outfile;    // Output file

static void error(string message) {
    cerr << message + '\n';
    exit(1);
}

static void vtk_init(p_vtk vtk) {
    vtk->version = (char*) malloc(256*sizeof(char));
    vtk->comment = (char*) malloc(256*sizeof(char));
    //vtk->dataset = (char*) malloc(256*sizeof(char));

    vtk->npoints = 0;
    vtk->npolygons = 0;
    vtk->polysize = 0;
    vtk->io_mode = NONE;
}

static p_vtk vtk_alloc() {
    p_vtk vtk = (p_vtk) calloc(1, sizeof(p_vtk));
    if (!vtk) return NULL;
    vtk_init(vtk);
    return vtk;
}

/* ----------------------------------------------------------------------
 * Read support functions
 * ---------------------------------------------------------------------- */
p_vtk vtk_open(const char *filename) {

    p_vtk vtk = vtk_alloc();
    if (!vtk) {
        error("Out of memory");
        return NULL;
    }
    if (filename == NULL) error("Invalid filename");

    // open file
    fs.open(filename, ifstream::in);
    if (!fs.is_open()) {
        error("Error opening file!");
    }

    return vtk;
}


// read a line from the filestream
static bool EMPTY_LINE = false;
stringstream* readLine() {
    string line;
    getline(fs, line);

    if (line.empty()) {
        EMPTY_LINE = true;
    } else {
        EMPTY_LINE = false;
    }
    return new stringstream(line);
}

void convert_vtk(p_vtk vtk, const char* outfilename) {
    string token;

    // File version and identifier
    stringstream *line;
    line = readLine();

    int count = 0;
    while ( getline(*line, token, ' ') ) {
        switch (count) {
            case 0:
                if (token != "#") error("VTK File invalid");
                break;
            case 1:
                if (token != "vtk") error("VTK File invalid");
                break;
            case 2:
                if (token != "DataFile") error("VTK File invalid");
                break;
            case 3:
                if (token != "Version") error("VTK File invalid");
                break;
            case 4:
                strcpy(vtk->version, token.c_str());
                //vtk->version = (token != "") ? token.c_str() : "";
                break;
            default:
                break;
        }
        count++;
    }

    // Header comment
    strcpy(vtk->comment, readLine()->str().c_str());
    //vtk->comment = readLine()->str();

    // File format (binary/ascii)
    token = readLine()->str();
    if (token == "ASCII") {
        vtk->io_mode = VTK_ASCII;
    } else if (token == "BINARY") {
        vtk->io_mode = VTK_BINARY;
    } else {
        error("Could not read VTK format type (binary/ascii)");
    }

    // Dataset
    line = readLine();
    getline(*line, token, ' ');
    if (token != "DATASET") error("Invalid dataset structure");

    getline(*line, token, ' ');
    //vtk->dataset = token;
    //vtk->dataset = (char*) malloc(256*sizeof(char));
    sprintf(vtk->dataset, "%s", token.c_str());
    //strcpy(vtk->dataset, token.c_str());


    // Echo header for checking
    if (debug) {
        cout << "VTK Version: " << vtk->version << '\n';
        cout << "Comment: " << vtk->comment << '\n';
        if (vtk->io_mode == VTK_ASCII) {
            cout << "Using ASCII decoding\n";
        } else if (vtk->io_mode == VTK_BINARY) {
            cout << "Using BINARY decoding\n";
        }
        cout << "Dataset: " << vtk->dataset << '\n';
    }

    /* Process data */
    // TODO split into read_points() and read_polygons
    // TODO check that we are reading floats
    // Dataset attributes
    line = readLine();
    getline(*line, token, ' ');
    if (token != "POINTS") error("Invalid dataset attributes");

    getline(*line, token, ' ');
    vtk->npoints = atol(token.c_str());

    getline(*line, token, ' ');
    if (token == "float") {
        vtk->data_mode = VTK_FLOAT;
    } else if (token == "double" ) {
        vtk->data_mode = VTK_DOUBLE;
    }

    if (debug) {
        cout << vtk->npoints << " POINTS ";
        if (vtk->data_mode == VTK_FLOAT) {
            cout << "float\n";
        } else if (vtk->data_mode == VTK_DOUBLE) {
            cout << "double\n";
        }

        cout << "Initializing JSON...\n";
    }

    /* Initialise JSON file */
    outfile.open (outfilename);

    string json_begin = "{\n";
    stringstream metadata;
    metadata << "\"metadata\":{\"formatVersion\":3,\"VTKVersion\":"
    << "\"" << vtk->version << "\""
    << ",\"vertices\":"
    << "\"" << vtk->npoints << "\""
    << ",\"faces\":"
    << "\"" << vtk->npolygons << "\""
    << "},\n";
    string json_metadata = metadata.str();
    string default_json = "\"scale\":1.0,\n\"materials\":[{\"DbgColor\":15658734,\"DbgIndex\":0,\"DbgName\":\"default\",\"vertexColors\": false}],\n\"vertices\": [";
    // print header
    outfile << json_begin << json_metadata << default_json;

    // Read points
    float p;
    long point_index = 0;
    line = readLine();
    while ( line->good() && !EMPTY_LINE ) {
        while ( getline(*line, token, ' ') ) {
            p = atof(token.c_str());
            if (!(point_index > 0)) {
                outfile << p;
            } else {
                outfile << "," << p;
            }
            point_index++;
        }
        delete(line);
        line = readLine();
    }

    outfile << "],\n";

    // Check that # of points is divisible by 3
    if (!(point_index % 3 == 0)) {
        error("Missing or extra points!\n");
    }

    /* Polygons */
    outfile << "\"faces\": [";

    // Read polygons
    int bitmask = 0;
    outfile << bitmask; // first bitmask
    long poly = 0;
    line = readLine();
    getline(*line, token, ' ');
    if (token != "POLYGONS") error("Unexpected token. Exiting...\n");
    getline(*line, token, ' ');
    vtk->npolygons = atol(token.c_str());
    getline(*line, token, ' ');
    vtk->polysize = atol(token.c_str());

    long npolys = 0;
    int nvertices;
    line = readLine();
    while ( line->good() && !EMPTY_LINE ) {
        getline(*line, token, ' ');
        nvertices = atoi(token.c_str());
        if (npolys > 0) {
          outfile << "," << bitmask; // padding
        }
          while (nvertices > 0) {
            getline(*line, token, ' ');
            poly = atoi(token.c_str());
            npolys++;
            outfile << "," << poly;
            nvertices--;
        }
        delete(line);
        line = readLine();
    }
    outfile << "],\n";

    string json_footer = "\"morphTargets\": [],\n\"normals\": [],\n\"colors\": [],\n\"uvs\": [[]]\n}\n";
    outfile << json_footer;

    // Report
    if (debug) {
        cout << "Parsed " << point_index/3 << "/" << vtk->npoints << " points\n";
        cout << "Parsed " << npolys << "/" << vtk->polysize << " polygon entries\n";
    }

    // deallocate and close
    outfile.close();

}


int main(int argc, const char * argv[]) {

    if (argc < 3)
	{
		cout << "Usage: vtk2json [vtkfile] [outfile]\n";
		return EXIT_FAILURE;
	}


    p_vtk vtk;
    vtk = vtk_open(argv[1]);
    convert_vtk(vtk, argv[2]);
    //vtk = vtk_open("/Users/frank/Desktop/20140411_6465_1316_res1024_full_vh.vtk");
    //convert_vtk(vtk, "/Users/frank/Desktop/out.json");

    fs.close();
    delete vtk;

    return 0;
}

