#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>

bool show_test_window = false;
float radius;
float reset ;
const int numCols = 14;
const int numRows = 18;




float posX = 0;
float posY = 0;
float posZ = 0;

float posX0 = 0;
float posY0 = 0;
float posZ0 = 0;

float incliZ = 0.1;

float data[numCols * numRows * 3];

float vecx;
float vecy;
float vecz;

float modul;

float v1;
float v2;
int ke = 50;
int kd = 1;
float restDist;
float llargada = 0.5f;
float llargadaShear;
float llargadaBending;

float mass = 1;
struct Particle
{

	glm::vec3 pos;
	glm::vec3 velocity;
	glm::vec3 antPos;
	glm::vec3 Forces;

};

Particle* totalParts;
Particle* AntParts;

namespace Sphere {
	extern void setupSphere(glm::vec3 pos = glm::vec3(0.f, 1.f, 0.f), float radius = 1.f);
	extern void cleanupSphere();
	extern void updateSphere(glm::vec3 pos, float radius = 1.f);
	extern void drawSphere();
}

namespace ClothMesh {
	extern void setupClothMesh();
	extern void cleanupClothMesh();
	extern void updateClothMesh(float* array_data);
	extern void drawClothMesh();
}

void GUI() {
	{	//FrameRate
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::SliderInt("KE", &ke,0,1000);
		ImGui::SliderInt("KD", &kd, 0, 10);
		ImGui::SliderFloat("Initial Rest Distance", &llargada, 0.3f,0.6);
		
		//TODO
	}

	// ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

void structuralForceH(Particle* part[], int i, int j) {
	glm::vec3 structuralForce;
	glm::vec3 p1;
	glm::vec3 p2;
	glm::vec3 vec;
	p1 = glm::vec3(totalParts[i*numCols + j].pos.x, totalParts[i*numCols + j].pos.y, totalParts[i*numCols + j].pos.z);

	p2 = glm::vec3(totalParts[(i)*numCols + (j + 1)].pos.x, totalParts[(i)*numCols + (j + 1)].pos.y, totalParts[(i)*numCols + (j + 1)].pos.z);
	vec = p1 - p2;

	modul = glm::length(vec);//modul del vector
	
	float var = -(ke*(modul - llargada) + kd * glm::dot((totalParts[i*numCols + j].velocity - totalParts[(i)*numCols + (j + 1)].velocity), (vec / modul)));
	structuralForce = var*(vec / modul);
	
	totalParts[i*numCols + j].Forces += structuralForce;
	totalParts[(i)*numCols + (j + 1)].Forces += -structuralForce;
}

void structuralForceV(Particle* part[], int i, int j) {
	//printf("%d\n", i);
	glm::vec3 structuralForce;
	glm::vec3 p1;
	glm::vec3 p2;
	glm::vec3 vec;
	p1 = glm::vec3(totalParts[i*numCols + j].pos.x, totalParts[i*numCols + j].pos.y, totalParts[i*numCols + j].pos.z);
	p2 = glm::vec3(totalParts[(i + 1)*numCols + j].pos.x, totalParts[(i + 1)*numCols + j].pos.y, totalParts[(i + 1)*numCols + j].pos.z);
	vec = p1 - p2;
	modul = glm::length(vec);//modul del vector
	if (modul >= restDist + (restDist * 10 / 100) || modul == NAN) {
		modul = restDist + (restDist * 10 / 100);
	}
	float var = -(ke*(modul - llargada) + kd * glm::dot((totalParts[i*numCols + j].velocity - totalParts[(i + 1)*numCols + j].velocity), (vec / modul)));
	structuralForce = var*(vec / modul);
	
	totalParts[i*numCols + j].Forces += structuralForce;
	totalParts[(i + 1)*numCols + j].Forces += -structuralForce;
	
}

void shearFroceLeft(Particle* part[], int i, int j) {
	glm::vec3 structuralForce;
	glm::vec3 p1;
	glm::vec3 p2;
	glm::vec3 vec;
	p1 = glm::vec3(totalParts[i*numCols + (numCols - j)].pos.x, totalParts[i*numCols + (numCols - j)].pos.y, totalParts[i*numCols + (numCols - j)].pos.z);

	p2 = glm::vec3(totalParts[(i + 1)*numCols + ((numCols - j) - 1)].pos.x, totalParts[(i + 1)*numCols + ((numCols - j) - 1)].pos.y, totalParts[(i + 1)*numCols + ((numCols - j) - 1)].pos.z);


	vec = p1 - p2;

	modul = glm::length(vec);//modul del vector
	if (modul >= llargadaShear - (llargadaShear * 10 / 100) || modul == NAN) {
		modul = llargadaShear - (llargadaShear * 10 / 100);
	}
	float var = -(ke*(modul - llargadaShear) + kd * glm::dot((totalParts[i*numCols + j].velocity - totalParts[(i + 1)*numCols + ((numCols - j) - 1)].velocity), (vec / modul)));
	structuralForce = var*(vec / modul);
	//printf("%f", structuralForce.y);

	totalParts[i*numCols + (numCols - j)].Forces += structuralForce;
	totalParts[(i + 1)*numCols + ((numCols - j) - 1)].Forces += -structuralForce;
}


void shearFroceRight(Particle* part[], int i, int j) {
	glm::vec3 structuralForce;
	glm::vec3 p1;
	glm::vec3 p2;
	glm::vec3 vec;
	p1 = glm::vec3(totalParts[i*numCols + j].pos.x, totalParts[i*numCols + j].pos.y, totalParts[i*numCols + j].pos.z);

	p2 = glm::vec3(totalParts[(i + 1)*numCols + (j + 1)].pos.x, totalParts[(i + 1)*numCols + (j + 1)].pos.y, totalParts[(i + 1)*numCols + (j + 1)].pos.z);


	vec = p1 - p2;

	modul = glm::length(vec);//modul del vector
	if (modul >= llargadaShear - (llargadaShear * 10 / 100) || modul == NAN) {
		modul = llargadaShear - (llargadaShear * 10 / 100);
	}
	float var = -(ke*(modul - llargadaShear) + kd * glm::dot((totalParts[i*numCols + j].velocity - totalParts[(i + 1)*numCols + (j + 1)].velocity), (vec / modul)));
	structuralForce = var*(vec / modul);
	//printf("%f", structuralForce.y);

	totalParts[i*numCols + j].Forces += structuralForce;
	totalParts[(i + 1)*numCols + (j + 1)].Forces += -structuralForce;
}

void bendingForceH(Particle* part[], int i, int j) {
	glm::vec3 structuralForce;
	glm::vec3 p1;
	glm::vec3 p2;
	glm::vec3 vec;
	p1 = glm::vec3(totalParts[i*numCols + j].pos.x, totalParts[i*numCols + j].pos.y, totalParts[i*numCols + j].pos.z);

	p2 = glm::vec3(totalParts[(i)*numCols + (j + 2)].pos.x, totalParts[(i)*numCols + (j + 2)].pos.y, totalParts[(i)*numCols + (j + 2)].pos.z);


	vec = p1 - p2;

	modul = glm::length(vec);//modul del vector
	
	float var = -(ke*(modul - llargadaBending) + kd * glm::dot((totalParts[i*numCols + j].velocity - totalParts[(i)*numCols + (j + 2)].velocity), (vec / modul)));
	structuralForce = var*(vec / modul);
	//printf("%f", structuralForce.y);

	totalParts[i*numCols + j].Forces += structuralForce;
	totalParts[(i)*numCols + (j + 2)].Forces += -structuralForce;
}


void bendingForceV(Particle* part[], int i, int j) {
	glm::vec3 structuralForce;
	glm::vec3 p1;
	glm::vec3 p2;
	glm::vec3 vec;
	p1 = glm::vec3(totalParts[i*numCols + j].pos.x, totalParts[i*numCols + j].pos.y, totalParts[i*numCols + j].pos.z);

	p2 = glm::vec3(totalParts[(i + 2)*numCols + (j)].pos.x, totalParts[(i + 2)*numCols + (j)].pos.y, totalParts[(i + 2)*numCols + (j)].pos.z);


	vec = p1 - p2;

	modul = glm::length(vec);//modul del vector
	
	float var = -(ke*(modul - llargadaBending) + kd * glm::dot((totalParts[i*numCols + j].velocity - totalParts[(i + 2)*numCols + (j)].velocity), (vec / modul)));
	structuralForce = var*(vec / modul);
	//printf("%f", structuralForce.y);

	totalParts[i*numCols + j].Forces += structuralForce;
	totalParts[(i + 2)*numCols + (j)].Forces += -structuralForce;
}

void posCorrectV(int i, int j) {
	if (i < 17 && j < 13) {
		glm::vec3 p1;
		glm::vec3 p2;
		glm::vec3 vec;
		p1 = glm::vec3(totalParts[i*numCols + j].pos);
		p2 = glm::vec3(totalParts[(i + 1)*numCols + j].pos);
		vec = p2 - p1;
		float mod = glm::length(vec);
		float max = restDist + (restDist * 10 / 100);
		if (mod >= max) {
			float diff = mod - (restDist + (restDist * 10 / 100));
			glm::vec3 unit = vec / mod;
			p1 = p1 + unit*(diff / 2);
			p2 = p2 - unit*(diff / 2);
			totalParts[i*numCols + j].pos = p1;
			totalParts[(i + 1)*numCols + j].pos = p2;
		}
	}
}

void posCorrectH(int i, int j) {
	if (i < 17 && j < 13) {
		glm::vec3 p1;
		glm::vec3 p2;
		glm::vec3 vec;
		p1 = glm::vec3(totalParts[i*numCols + j].pos);
		p2 = glm::vec3(totalParts[i* numCols + (j + 1)].pos);
		vec = p2 - p1;
		float mod = glm::length(vec);
		
		if (mod >= restDist + (restDist * 10 / 100)) {
			float diff = mod - (restDist + (restDist * 10 / 100));
			glm::vec3 unit = vec / mod;
			p1 = p1 + unit*(diff / 2);
			p2 = p2 - unit*(diff / 2);
			totalParts[i*numCols + j].pos = p1;
			totalParts[i* numCols + (j + 1)].pos = p2;
		}
	}
}

void posCorrectShearRight(int i, int j) {
	if (i < 17 && j < 13) {
		glm::vec3 p1;
		glm::vec3 p2;
		glm::vec3 vec;
		p1 = glm::vec3(totalParts[i*numCols + j].pos);
		p2 = glm::vec3(totalParts[(i + 1)*numCols + (j+1)].pos);
		vec = p1 - p2;
		float mod = glm::length(vec);
		if (mod >= llargadaShear + (llargadaShear * 10 / 100)) {
			glm::vec3 unit = vec / mod;
			float diff = mod - llargadaShear + (llargadaShear * 10 / 100);
			p1 = p1 + unit*(diff / 2);
			p2 = p2 - unit*(diff / 2);
			totalParts[i*numCols + (j)].pos = p1;
			totalParts[(i + 1)*numCols + (j + 1)].pos = p2;
		}
	}
}

void posCorrectShearLeft(int i, int j) {
	if (i < 17 && j < 13) {
		glm::vec3 p1;
		glm::vec3 p2;
		glm::vec3 vec;
		p1 = glm::vec3(totalParts[i*numCols + (numCols -j)].pos);
		p2 = glm::vec3(totalParts[(i + 1)*numCols + ((numCols - j) - 1)].pos);
		vec = p1 - p2;
		float mod = glm::length(vec);
		if (mod >= llargadaShear + (llargadaShear * 10 / 100)) {
			glm::vec3 unit = vec / mod;
			float diff = mod - llargadaShear + (llargadaShear * 10 / 100);
			p1 = p1 + unit*(diff / 2);
			p2 = p2 - unit*(diff / 2);
			totalParts[i*numCols + (numCols - j)].pos = p1;
			totalParts[(i + 1)*numCols + ((numCols - j) - 1)].pos = p2;
		}
	}
}

void PhysicsInit() {
	//TODO
	//ke = 5;//duresa de la molla
	//kd = 1;//restriccio del moviment
	
	
	restDist = llargada;
	llargadaShear = sqrt((llargada*llargada) + (llargada*llargada));
	
	llargadaBending = llargada * 2;
	reset = 5;
	radius = ((float)rand() / RAND_MAX)*1.5+0.5;
	//printf("%f", radius);
	glm::vec3 vector = glm::vec3(((float)rand() / RAND_MAX) * 5 - 5, ((float)rand() / RAND_MAX) * 10, ((float)rand() / RAND_MAX) * 5 - 5);
	if (vector.y >= 10 - radius) {
		vector.y -= radius;
	}
	else if (vector.y <= 0 + radius) {
		vector.y += radius;
	}
	if (vector.x >= 5 - radius) {
		vector.x -= radius;
	}
	else if (vector.x <= -5 + radius) {
		vector.x += radius;
	}
	if (vector.z >= 5 - radius) {
		vector.z -= radius;
	}
	else if (vector.z <= -5 + radius) {
		vector.z += radius;
	}

	Sphere::updateSphere(vector,radius);
	/*for (int i = 0; i < numCols*numRows; i++)
	{
		printf("%d\n", 3*i +0);
		printf("%d\n", 3*i + 1);
		printf("%d\n", 3*i + 2);
		data[ 3+i + 0] = -4.5*i*0.5;
		data[ 3*i + 1] = -4.5*i*0.5;
		data[ 3*i + 2] = -4.5*i*0.5;
	}*/
	totalParts = new Particle[14 * 18];
	
	for (int i = 0; i < (numRows); ++i) {
		
		for (int j = 0; j < (numCols); ++j) {
			

			totalParts[i*numCols + j].pos = glm::vec3(j*restDist - 5, 8, i*restDist - 5 );
			//printf("CurPos: %f\n", totalParts[i*numCols + j].pos.x);
			totalParts[i*numCols + j].antPos = glm::vec3(totalParts[i*numCols + j].pos.x, totalParts[i*numCols + j].pos.y, totalParts[i*numCols + j].pos.z);
			//printf("AntPos: %f\n", totalParts[i*numCols + j].antPos.x);

			totalParts[i*numCols + j].Forces.x = 0;
			totalParts[i*numCols + j].Forces.y = 0;
			totalParts[i*numCols + j].Forces.z = 0;

			totalParts[i*numCols + j].velocity.x = 0;
			totalParts[i*numCols + j].velocity.y = 0;
			totalParts[i*numCols + j].velocity.z = 0;

			data[3 * (i*numCols + j) + 0] = totalParts[i*numCols + j].pos.x;
			data[3 * (i*numCols + j) + 1] = totalParts[i*numCols + j].pos.y;
			data[3 * (i*numCols + j) + 2] = totalParts[i*numCols + j].pos.z;
			
			
		}
	}
	
	posX0 = data[0];
	posY0 = data[1];
	posZ0 = data[2];

	posX = data[39];
	posY = data[40];
	posZ = data[41];
	
	ClothMesh::updateClothMesh(data);
	
}

void PhysicsUpdate(float dt) {
	
	reset -= dt;
	if (reset <= 0) {
		PhysicsInit();
	}
	
	//TODO
	for (int i = 0; i < (numRows); i++) {
		//printf("%d", i);
		for (int j = 0; j < (numCols); j++) {
			//correcio de posicio
			posCorrectV(i, j);
			posCorrectH(i, j);
			//posCorrectShearLeft(i, j);
			//posCorrectShearRight(i, j);

			//calcul posicio Verlet
			glm::vec3 temp = totalParts[i*numCols + j].pos;
			

			totalParts[i*numCols + j].pos.x = totalParts[i*numCols + j].pos.x + (totalParts[i*numCols + j].pos.x - totalParts[i*numCols + j].antPos.x) + (totalParts[i*numCols + j].Forces.x / mass)*(dt*dt);

			totalParts[i*numCols + j].pos.y = totalParts[i*numCols + j].pos.y + (totalParts[i*numCols + j].pos.y - totalParts[i*numCols + j].antPos.y) + (totalParts[i*numCols + j].Forces.y / mass)*(dt*dt);

			totalParts[i*numCols + j].pos.z = totalParts[i*numCols + j].pos.z + (totalParts[i*numCols + j].pos.z - totalParts[i*numCols + j].antPos.z) + (totalParts[i*numCols + j].Forces.z / mass)*(dt*dt);
			totalParts[0].pos = glm::vec3(posX0, posY0, posZ0);
			totalParts[13].pos = glm::vec3(posX, posY, posZ);
			totalParts[i*numCols + j].velocity = (totalParts[i*numCols + j].pos - temp) / dt;
			
			
			totalParts[i*numCols + j].antPos = temp;
			totalParts[i*numCols + j].Forces = glm::vec3(0, -9.8, 0);

			//Structural Vertical
			if (j < numCols-1 && i < numRows -1) {
				structuralForceV(&totalParts, i, j);
			}
			//Structural Horizontal
			if (j < numCols -1  && i < numRows - 1) {
				structuralForceH(&totalParts, i, j);
			}
				//Shear Forces Left
			if (j < 12 && i < 17) {
				//shearFroceLeft(&totalParts, i, j);
			}
			//Shear Forces Right
			if (j < 12 && i < 17) {

				//shearFroceRight(&totalParts, i, j);

			}
			//Bending Force Vertical
			if (j < 13 && i < 15) {
				bendingForceV(&totalParts, i, j);
			}
			//Bending Force Horizontal
			if (j < 11 && i < 17) {
				bendingForceH(&totalParts, i, j);
			}

				


			data[3 * (i*numCols + j) + 0] = totalParts[i*numCols + j].pos.x;
			data[3 * (i*numCols + j) + 1] = totalParts[i*numCols + j].pos.y;
			data[3 * (i*numCols + j) + 2] = totalParts[i*numCols + j].pos.z;

			if (data[3 * (i*numCols + j) + 2] <= -5) {
				data[3 * (i*numCols + j) + 2] = -5;
			}
		}
	}









		/*data[0] = posX0;
		data[1] = posY0;
		data[2] = posZ0;

		data[39] = posX;
		data[40] = posY;
		data[41] = posZ;*/
		ClothMesh::updateClothMesh(data);



	}

void PhysicsCleanup() {
	//TODO
	ClothMesh::cleanupClothMesh();
	delete[] totalParts;
}