#include "RenderList.h"

#include "gles.h"
#include "RenderChunk.h"
#include "RendererBackend.h"
#include "Tesselator.h"


RenderList::RenderList()
	:	inited(false),
	rendered(false)
{
	lists = new int[MAX_NUM_OBJECTS];
	rlists = new RenderChunk[MAX_NUM_OBJECTS];

	for (int i = 0; i < MAX_NUM_OBJECTS; ++i)
		rlists[i].vboId = -1;
}

RenderList::~RenderList() {
	delete[] lists;
	delete[] rlists;
}

void RenderList::init(double xOff, double yOff, double zOff) {
	inited = true;
	listIndex = 0;

	this->xOff = xOff;
	this->yOff = yOff;
	this->zOff = zOff;
}

void RenderList::add(int list) {
	lists[listIndex] = list;
	if (listIndex == MAX_NUM_OBJECTS) /*lists.remaining() == 0)*/ render();
}

void RenderList::addR(const RenderChunk& chunk) {
	rlists[listIndex] = chunk;
}

void RenderList::render() {

	if (!inited) return;
	if (!rendered) {
		bufferLimit = listIndex;
		listIndex = 0;
		rendered = true;
	}
	if (listIndex < bufferLimit) {
		glPushMatrix2();

		#ifndef USE_VBO
			for (int i = 0; i < bufferLimit; ++i) {
				RenderChunk& rc = rlists[i];
				double rx = rc.pos.x - xOff;
				double ry = rc.pos.y - yOff;
				double rz = rc.pos.z - zOff;

				glPushMatrix2();
				glTranslatef2((float)rx, (float)ry, (float)rz);
				glCallList(lists[i]);
				glPopMatrix2();
			}
		#else
			renderChunks();
		#endif/*!USE_VBO*/

		glPopMatrix2();
	}
}

void RenderList::renderChunks() {
	const int Stride = VertexSizeBytes;

	for (int i = 0; i < bufferLimit; ++i) {
		RenderChunk& rc = rlists[i];

		double rx = rc.pos.x - xOff;
		double ry = rc.pos.y - yOff;
		double rz = rc.pos.z - zOff;
		glPushMatrix2();
		glTranslatef2((float)rx, (float)ry, (float)rz);
		rendererBackend().drawVertexBuffer(rc.vboId, rc.vertexCount, Stride, RENDERER_VERTEX_FORMAT_VTC, GL_TRIANGLES);

		glPopMatrix2();
	}
}

void RenderList::clear() {
	inited = false;
	rendered = false;
}
