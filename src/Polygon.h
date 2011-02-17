/*
 * Polygon.h
 *
 *  Created on: Feb 2, 2009
 *      Author: njoubert
 */

#ifndef POLYGON_H_
#define POLYGON_H_

#include "global.h"
#include "Vertex.h"

class Polygon {
public:
    Polygon();
    Polygon(const Polygon& poly); // copy constructor
    Polygon& operator =(const Polygon& poly); // assignment of a Polygon

    /* Loads the first polygon found in an OBJ file */
    Polygon(string objfile);

    /* Draws the given polygon using OpenGL. */
    void draw();
    /* Draws the given polygon, linearly interpolated at t in interval [0,1] */
    void draw(double t);

	void drawBoundingBox(double t);
	
    /* Adds a vertex to the end of this polygon */
    void addVertex(Vertex * v);

    /* Writes out the polygon as an OBJ file. */
    void writeAsOBJ(string filename);
    /* Writes out the polygon interpolated at t in inverval [0,1] as an OBJ. */
    void writeAsOBJ(string filename, double t);

	/* calculates the distance between two points given the points*/
	double distance(const double x1, const double y1, const double x2, const double y2);
	
	/* finds the vertex in _vertices that is closest to v */
	Vertex * closest(Vertex v); 
	
	/* finds the bounds of the boundary box for the polygon interpolated at t */
	void findBounds(double t); 
	
	void morphCopy();
	
	vector<Vertex*> findVerticesIn(double xmin,double xmax,double ymin,double ymax);
	
	void circleVertices(double r,vector<Vertex*> picked,double t);

private:
    bool _parseLine(string, vector<Vertex> &);
    vector<Vertex> _vertices;
	double _xmin,_xmax,_ymin,_ymax;
};

#endif /* POLYGON_H_ */
