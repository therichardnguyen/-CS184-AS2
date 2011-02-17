/*
 * Vertex.cpp
 *
 *  Created on: Feb 2, 2009
 *      Author: njoubert
 */

#include "Vertex.h"

Vertex::Vertex(): _startPos(0.0,0.0), _endPos(0.0,0.0) {
    // TODO Auto-generated constructor stub

}

Vertex::Vertex(vec2 p): _startPos(p), _endPos(0.0,0.0) {

}
Vertex::Vertex(double x, double y): _startPos(x,y), _endPos(0.0,0.0) {

}

vec2 Vertex::getPos() {
    return _startPos;
}

vec2 Vertex::getPos(double t) {
	vec2 interPos = _startPos*(1-t) + _endPos*(t);
    return interPos;
}

void Vertex::setStartPos(vec2 p) {
    _startPos = p;
}

void Vertex::setEndPos(vec2 p) {
    _endPos = p;
}
