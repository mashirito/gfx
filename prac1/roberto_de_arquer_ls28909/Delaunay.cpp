#include <iostream>
#include <list>
#include "Delaunay.h"


void triangulateMesh(Mesh &mesh){

	superTriangle(mesh);

	for (int i = 0; i < mesh.vertices.size() - 3; i++){
		addVertex(mesh, i);
		if (i % 100 == 0) std::cout << i << " of " << mesh.vertices.size() << " completed.\n";
	}

	for (int i = 0; i < mesh.faces.size(); i++){
		if ((mesh.faces[i].a == mesh.vertices.size() - 3) || (mesh.faces[i].a == mesh.vertices.size() - 2) || (mesh.faces[i].a == mesh.vertices.size() - 1) ||
			(mesh.faces[i].b == mesh.vertices.size() - 3) || (mesh.faces[i].b == mesh.vertices.size() - 2) || (mesh.faces[i].b == mesh.vertices.size() - 1) ||
			(mesh.faces[i].c == mesh.vertices.size() - 3) || (mesh.faces[i].c == mesh.vertices.size() - 2) || (mesh.faces[i].c == mesh.vertices.size() - 1))
		{
			mesh.faces.erase(mesh.faces.begin() + i);
			i = i - 1;
		}
	}
}

void superTriangle(Mesh &mesh){

	double
		xmin = mesh.vertices[0].x,
		xmax = xmin,
		ymin = mesh.vertices[0].y,
		ymax = ymin;

	for (int i = 1; i < mesh.vertices.size(); i++){
		if (mesh.vertices[i].x < xmin) xmin = mesh.vertices[i].x;
		if (mesh.vertices[i].x > xmax) xmax = mesh.vertices[i].x;
		if (mesh.vertices[i].y < ymin) ymin = mesh.vertices[i].y;
		if (mesh.vertices[i].y > ymax) ymax = mesh.vertices[i].y;
	}

	double
		dx = xmax - xmin,
		dy = ymax - ymin,
		dmax = (dx > dy) ? dx : dy,
		xmid = (xmax + xmin) / 2.0,
		ymid = (ymax + ymin) / 2.0;

	Vertex
		v1{ xmid - 20 * dmax, ymid - dmax, 0.0 },
		v2{ xmid + 20 * dmax, ymid - dmax, 0.0 },
		v3{ xmid, ymid + 20 * dmax, 0.0 };

	mesh.vertices.push_back(v1);
	mesh.vertices.push_back(v2);
	mesh.vertices.push_back(v3);
	mesh.faces.push_back(Face{ mesh.vertices.size() - 3, mesh.vertices.size() - 2, mesh.vertices.size() - 1 });

}

//Implement this function
void addVertex(Mesh &mesh, const int vertexIndex){
	//Vector donde guardaremos los edges que corresponden a los vertices que incumplen Delaunay
	std::vector< Edge > edges;

	//Recorremos todas las caras de la malla
	for (int i = 0; i < mesh.faces.size(); i++) {
		//Si los vertices de esa cara de la malla han dejado de cumplir la condición de Delaunay
		if (isInsideCircumCircle(mesh.vertices[mesh.faces[i].a], mesh.vertices[mesh.faces[i].b], mesh.vertices[mesh.faces[i].c], mesh.vertices[vertexIndex])) {
			//Guardamos las arestas de esa cara para eliminar las duplicadas posteriormente
			Edge aux;
			aux.v1 = mesh.faces[i].a;
			aux.v2 = mesh.faces[i].b;
			edges.push_back(aux);

			aux.v1 = mesh.faces[i].b;
			aux.v2 = mesh.faces[i].c;
			edges.push_back(aux);

			aux.v1 = mesh.faces[i].c;
			aux.v2 = mesh.faces[i].a;
			edges.push_back(aux);
			//Eliminamos el triangulo de la malla
			mesh.faces.erase(mesh.faces.begin() + i);
			i--;
		}
	}
	//Eliminamos las aretas repetidas de nuestro vector de Edges
	for (int i = 0; i < edges.size(); i++) {
		for (int j = 0; j < edges.size(); j++) {
			if (i != j) {
				if (((edges[i].v1 == edges[j].v2) && (edges[i].v2 == edges[j].v1)) || ((edges[i].v1 == edges[j].v1) && (edges[i].v2 == edges[j].v2))) {
					edges.erase(edges.begin() + j);
					edges.erase(edges.begin() + i);
					i--;
				}
			}
		}
	}

	//Unimos los vertices de cada Edge al nuevo vértice indicado
	for (int i = 0; i < edges.size(); i++) {
		Face face;

		face.a = edges[i].v1;
		face.b = edges[i].v2;
		face.c = vertexIndex;
		//Hacemos un push_back de la nueva cara
		mesh.faces.push_back(face);
	}

}

bool isInsideCircumCircle(const Vertex A, const Vertex B, const Vertex C, const Vertex &point){
	double m1, m2, mx1, mx2, my1, my2, xc, yc, r;
	double dx, dy, rsqr, drsqr;

	if (abs(A.y - B.y) < EPSILON && abs(B.y - C.y) < EPSILON)
		return(false);
	if (abs(B.y - A.y) < EPSILON){
		m2 = -(C.x - B.x) / (C.y - B.y);
		mx2 = (B.x + C.x) / 2.0;
		my2 = (B.y + C.y) / 2.0;
		xc = (B.x + A.x) / 2.0;
		yc = m2 * (xc - mx2) + my2;
	}
	else if (abs(C.y - B.y) < EPSILON){
		m1 = -(B.x - A.x) / (B.y - A.y);
		mx1 = (A.x + B.x) / 2.0;
		my1 = (A.y + B.y) / 2.0;
		xc = (C.x + B.x) / 2.0;
		yc = m1 * (xc - mx1) + my1;
	}
	else{
		m1 = -(B.x - A.x) / (B.y - A.y);
		m2 = -(C.x - B.x) / (C.y - B.y);
		mx1 = (A.x + B.x) / 2.0;
		mx2 = (B.x + C.x) / 2.0;
		my1 = (A.y + B.y) / 2.0;
		my2 = (B.y + C.y) / 2.0;
		xc = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
		yc = m1 * (xc - mx1) + my1;
	}
	dx = B.x - xc;
	dy = B.y - yc;
	rsqr = dx * dx + dy * dy;
	dx = point.x - xc;
	dy = point.y - yc;
	drsqr = dx * dx + dy * dy;
	return((drsqr <= rsqr) ? true : false);

}






