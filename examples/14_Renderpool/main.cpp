#include <TinyEngine/TinyEngine>
#include <TinyEngine/camera>

#include <unordered_set>

#include "renderpool.cpp"
#include "chunk.h"

using namespace glm;

int main( int argc, char* args[] ) {

	srand(time(NULL));

	Tiny::view.vsync = false;
	Tiny::benchmark = true;
	Tiny::window("Example Window", 1200, 800);

  cam::near = -500.0f;
  cam::far = 500.0f;
	cam::rot = 45.0f;
	cam::roty = 45.0f;
  cam::look = vec3(32, 0, 32);
  cam::init(3.5, cam::ORTHO);
	cam::update();

	const int N = 5*5*5*6;	//6 Face Orientations per Chunk

	Chunk::LOD = 1;
	Chunk::QUAD = 3600;
	//Chunk::LOD = 2;
	//Chunk::QUAD = 800;
	//Chunk::LOD = 4;
	//Chunk::QUAD = 200;
	//Chunk::LOD = 8;
	//Chunk::QUAD = 50;

	std::unordered_set<int> groups;

	Renderpool<Vertex> vertpool(Chunk::QUAD, N);

	Chunk chunk;
	vector<Chunk> chunks;

	std::cout<<"Meshing ";
	timer::benchmark<std::chrono::microseconds>([&](){

	for(int i = 0; i < 5; i++)
	for(int j = 0; j < 5; j++)
	for(int k = 0; k < 5; k++){

		chunk.randomize();
		chunk.pos = ivec3(i,j,k);
		chunks.push_back(chunk);

		chunkmesh::greedypool(&chunks.back(), &vertpool);

	}

	groups.clear();
	if(cam::pos.x < 0) groups.insert(0);
	if(cam::pos.x > 0) groups.insert(1);
	if(cam::pos.y < 0) groups.insert(2);
	if(cam::pos.y > 0) groups.insert(3);
	if(cam::pos.z < 0) groups.insert(4);
	if(cam::pos.z > 0) groups.insert(5);
	vertpool.mask([&](DAIC& cmd){
		return groups.contains(cmd.group);
	});

	vertpool.order([&](const DAIC& a, const DAIC& b){
		if(dot(b.pos - a.pos, cam::pos) < 0) return true;
		if(dot(b.pos - a.pos, cam::pos) > 0) return false;
		return (a.baseVert < b.baseVert);
	});

	vertpool.update();

});

	//Render Pass

	Shader shader({"shader/default.vs", "shader/default.fs"}, {"in_Position", "in_Normal", "in_Color"});

	vec3 oldpos;
	vec3 newpos;

	oldpos.x = (cam::pos.x > 0)?1:-1;
	oldpos.y = (cam::pos.y > 0)?1:-1;
	oldpos.z = (cam::pos.z > 0)?1:-1;
	if(cam::pos.x == 0) oldpos.x = 0;
	if(cam::pos.y == 0) oldpos.y = 0;
	if(cam::pos.z == 0) oldpos.z = 0;

	Tiny::event.handler = [&](){ cam::handler();

		if(cam::moved){

			newpos.x = (cam::pos.x > 0)?1:-1;
			newpos.y = (cam::pos.y > 0)?1:-1;
			newpos.z = (cam::pos.z > 0)?1:-1;
			if(cam::pos.x == 0) newpos.x = 0;
			if(cam::pos.y == 0) newpos.y = 0;
			if(cam::pos.z == 0) newpos.z = 0;

			if(!all(equal(oldpos, newpos))){

				groups.clear();
				if(cam::pos.x < 0) groups.insert(0);
				if(cam::pos.x > 0) groups.insert(1);
				if(cam::pos.y < 0) groups.insert(2);
				if(cam::pos.y > 0) groups.insert(3);
				if(cam::pos.z < 0) groups.insert(4);
				if(cam::pos.z > 0) groups.insert(5);
				vertpool.mask([&](DAIC& cmd){
					return groups.contains(cmd.group);
				});

			}

			oldpos = newpos;

		}

	};

	Tiny::view.interface = [&](){ /* ... */ };

	Tiny::view.pipeline = [&](){

		Tiny::view.target(glm::vec3(0));	//Clear Screen to white
		shader.use();
		shader.uniform("vp", cam::vp);

		vertpool.render();

	};

	Tiny::loop([&](){ /* ... */

		if(Tiny::benchmark) std::cout<<Tiny::average<<std::endl;

		int r;

	//	int t = 0;

		for(int i = 0 ; i < 50; i++){

			r = rand()%chunks.size();
			chunks[r].randomize();

			for(int d = 0; d < 6; d++)
				vertpool.unsection(chunks[r].faces[d]);

			chunkmesh::greedypool(&chunks[r], &vertpool);

		}

//		std::cout<<t<<std::endl;

	//	std::cout<<"Sort Pass ";
	//	timer::benchmark<std::chrono::microseconds>([&](){

		vertpool.mask([&](DAIC& cmd){
			return groups.contains(cmd.group);
		});

		vertpool.order([&](const DAIC& a, const DAIC& b){
			if(dot(b.pos - a.pos, cam::pos) < 0) return true;
			if(dot(b.pos - a.pos, cam::pos) > 0) return false;
			return (a.baseVert < b.baseVert);
		});

	//	});

	if(chunks.size() == 0) Tiny::event.quit = true;

	});

	delete[] chunk.data;

	Tiny::quit();
	return 0;

}
