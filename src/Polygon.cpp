/*
 * Polygon.cpp
 *
 *  Created on: Feb 2, 2009
 *      Author: njoubert
 */

#include "Polygon.h"

Polygon::Polygon() {
    // Leave it empty.
}

/**
 * This parses an OBJ file for the first polygon available in it.
 */
Polygon::Polygon(string objfile) {
    // Leave this as it is.
    std::cout << "Parsing OBJ file " << objfile << std::endl;

    vector<Vertex> tempVerts;
    ifstream inFile(objfile.c_str(), ifstream::in);
    if (!inFile) {
        std::cout << "Could not open given obj file " << objfile << std::endl;
    }
    while (inFile.good()) {
        string line;
        getline(inFile, line);
        if (!_parseLine(line, tempVerts)) {
            std::cout << "Failed to parse OBJ file." << std::endl;
            break;
        }
        if (_vertices.size() > 0) // take the first face in the file.
            break;
    }
    inFile.close();

    std::cout << "Parsed an OBJ file with " << _vertices.size() << " vertices."
            << endl;
}


bool Polygon::_parseLine(string line, vector<Vertex> & temp) {
    string operand;
    bool success = true;

    if (line.empty())
        return true;
    stringstream ss(stringstream::in | stringstream::out);
    ss.str(line);
    ss >> operand;

    if (operand[0] == '#') {

        return true;

    } else if (operand == "v") {
        double x, y;
        ss >> x >> y;
        temp.push_back(Vertex(x, y));

    } else if (operand == "f") {
        while (!ss.eof()) {
            int i;
            ss >> i;
            addVertex(&temp[i - 1]); // copy vertex in to polygon
        }

    } else {
		cout << operand;
        cout << "Unknown operand in scene file, skipping line: " << operand
                << endl;

    }

    if (ss.fail()) {
        std::cout
                << "The bad bit of the input file's line's stringstream is set! Couldn't parse:"
                << std::endl;
        std::cout << "  " << line << std::endl;
        success = false;
    }

    return success;
}

void Polygon::draw() {
    if (1 > _vertices.size())
        return;
	glColor3f(0.5,0.5,0.5);
	glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINE_LOOP);
    for (vector<Vertex>::iterator it = _vertices.begin(); it
            != _vertices.end(); it++)
        glVertex2d(it->getPos()[0], it->getPos()[1]);
    glEnd();
	glDisable(GL_LINE_STIPPLE);
}

void Polygon::draw(double t) {
	glColor3f(1.0,1.0,1.0);
	glBegin(GL_LINE_LOOP);
	for (vector<Vertex>::iterator it = _vertices.begin(); it
		!= _vertices.end(); it++)
		glVertex2d(it->getPos(t)[0], it->getPos(t)[1]);
	glEnd();	
}

void Polygon::drawBoundingBox(double t) {
	findBounds(t);
	glColor3f(0.0,1.0,0.0);
	glLineStipple(1, (short) 0x8888);
	glEnable(GL_LINE_STIPPLE);
	glBegin(GL_LINE_LOOP);
	glVertex2d(_xmin,_ymax);
	glVertex2d(_xmax,_ymax);
	glVertex2d(_xmax,_ymin);
	glVertex2d(_xmin,_ymin);
	glEnd();
	glDisable(GL_LINE_STIPPLE);
	glLineStipple(1,(short) 0x5555);	
}

void Polygon::addVertex(Vertex *v) {
    _vertices.push_back(*v);
}

void Polygon::writeAsOBJ(string filename) {
    writeAsOBJ(filename, 0);
}

void Polygon::writeAsOBJ(string filename, double t) {

    	unsigned int i=1;
		ofstream obj(filename.c_str(), ios::out);
		for (vector<Vertex>::iterator it = _vertices.begin();it
				!= _vertices.end(); it++) 
			obj << "v " << it->getPos(t)[0] << " " << it->getPos(t)[1] << endl;
 		obj << "f";
        for (i=1;i<=_vertices.size(); i++) {
            obj << " " << i ;
        }
        obj << endl;
		obj.close();

}
double Polygon::distance(double x1, double y1, double x2, double y2) {
	return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

Vertex * Polygon::closest(Vertex v) {
	Vertex * rtn = NULL;
 	double d = numeric_limits<double>::max( );
	unsigned int i;
	for(i=0;i<_vertices.size();i++) {
		if (distance(v.getPos()[0], v.getPos()[1],_vertices.at(i).getPos(1.0)[0],_vertices.at(i).getPos(1.0)[1]) <= d) {
			d = distance(v.getPos()[0], v.getPos()[1],_vertices.at(i).getPos(1.0)[0],_vertices.at(i).getPos(1.0)[1]);
			rtn = &(_vertices.at(i));
		}
	}
	return rtn;
}

void Polygon::morphCopy() {
	findBounds(0);
	for(vector<Vertex>::iterator it = _vertices.begin();it != _vertices.end();it++) {
		it->setEndPos(it->getPos());
	}
}

void Polygon::findBounds(double t) {
	_xmin = _ymin = numeric_limits<double>::min();
	_xmax = _ymax = numeric_limits<double>::max();
	for(vector<Vertex>::iterator it = _vertices.begin();it != _vertices.end();it++) {
		if (it->getPos(t)[0]>_xmin)
			_xmin = it->getPos(t)[0];
		if (it->getPos(t)[0]<_xmax)
			_xmax = it->getPos(t)[0];
		if (it->getPos(t)[1]>_ymin)
			_ymin = it->getPos(t)[1];
		if (it->getPos(t)[1]<_ymax)
			_ymax = it->getPos(t)[1];
	}
}

vector<Vertex*> Polygon::findVerticesIn(double xmin,double xmax,double ymin,double ymax) {
	vector<Vertex*> rtn;
	unsigned int i;
	for(i=0;i<_vertices.size();i++) {
		vec2 v = _vertices.at(i).getPos(1.0);
		if (v[0]<=xmax && v[0]>=xmin && v[1]<=ymax && v[1]>=ymin) {
			rtn.push_back(&(_vertices.at(i)));
		}
	}
	return rtn;
}

void Polygon::circleVertices(double r,vector<Vertex*> picked,double t) {
	unsigned int i,j;
	double a = 0;
	glColor3f(0,1.0,1.0);
	for (i=0;i<_vertices.size();i++) {
		Vertex * it = &(_vertices.at(i));
		glBegin(GL_LINE_LOOP);
		for(j=0;j<picked.size();j++) {
			if (it == picked.at(j)) {
				glColor3f(1.0,0.0,1.0);
				break;
			} else {
				glColor3f(0,1.0,1.0);
			}
		}
		for(a=0;a<=2*PI;a+=PI/20) {
			glVertex2d(it->getPos(t)[0]+(r*cos(a)), it->getPos(t)[1]+(r*sin(a)));
		}
		glEnd();
	}
}

