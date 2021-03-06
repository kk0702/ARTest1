#pragma once


#include <tchar.h>
#include <windows.h>
#define D3D_DEBUG_INFO
#include <stdlib.h>
#include <math.h>
#include <d3dx9.h>
#include <XAudio2.h>
#include <vector>

#include "../include/WindowManager.h"
#include "../include/ars.h"
#include "arstest.h"


//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------


void subtract_mask(Texture* result, Texture* bg, Texture* src, DWORD border);
const unsigned int sizex = 640; 
const unsigned int sizey = 480;

UINT MainLoop(WindowManager *winmgr)
{
	//ShowDebugWindow();

	//for debug(1/2)
	//Window window2;
	//winmgr->RegisterWindow(&window2);

	//ARSG arsgd(window2.hWnd, sizex, sizey, true);
	//Texture debug(&arsgd, sizex, sizey);
	//debug.SetDrawMode(true);
	
	Window window;
	winmgr->RegisterWindow(&window);

	ARSG g(window.hWnd, sizex, sizey, true);
	g.SetBackgroundColor(255,0,0,0);

	Light light(&g);	
	g.Register(&light);

	ARSD d;
	d.Init();
	d.AttachCam(0);
	d.StartGraph();
	
	Texture hitArea(&g,sizex,sizey);
	Texture stored (&g,sizex,sizey);
	Texture source (&g,sizex,sizey);
	source.SetDrawMode(TRUE);
	g.Register(&source);

	Touchable ball(&g, L"ball.x");	
	ball.SetScale(3.0f, 3.0f, 3.0f);
	ball.SetPosition(0.0f, 0.0f, 0.0f,GL_ABSOLUTE);		
	g.Register(&ball);

	InputHandler *keyIn = window.GetInputHandler();
	
	while(!d.GetCamImage(&stored));

	while (!winmgr->WaitingForTermination()){
		if (keyIn->GetKeyTrig('A'))
			d.GetCamImage(&stored);
		d.GetCamImage(&source);
		if (keyIn->GetKeyTrig('Q')) break;
			
		subtract_mask(&hitArea,&stored,&source,0x20202020);	
	
		
		ball.react(&hitArea);
		ball.move();

		//for debug(2/2)
		//debug = hitArea;
		//arsgd.Draw(&debug);
		g.Draw();
		
	}
	d.StopGraph();
	return 0;
}

inline void subtract_mask(Texture* result, Texture* backgrnd, Texture* src, DWORD border)
{
	ARSC::diff(result,backgrnd,src,border);
	ARSC::monochrome(result,result);
	ARSC::thresholding(result,result,border);
}


inline bool Touchable::get_overlapping_center(Texture* hitArea, int *pGx, int *pGy, unsigned int threshold)
{	
	static Texture txtr;
	ARSG* g = GetARSG(); 
	txtr.Init(g,sizex,sizey);

	unsigned int pixel_count;

	g->Draw(this,&txtr);
	ARSC::and(&txtr, &txtr, hitArea, 0x10101010);

	ARSC::getCG(pGx, pGy, &pixel_count, &txtr);	
	return pixel_count > threshold;
}

inline void Touchable::react(Texture* _hitArea)//�����蔻��
{
	int gx,gy;
	bool overlapping = get_overlapping_center(_hitArea, &gx, &gy,100);

	VECTOR2D c;		
	GetARSG()->Convert3Dto2D(&c, GetPosition());

	switch (state) {
		case OUT_TOUCH:
			if (overlapping) {
				vx = (c.x - gx) * 0.05f;
				vy = -(c.y - gy) * 0.05f;
				state = IN_TOUCH;
			}
			break;
		case IN_TOUCH:
			SetRotationZ(-0.1f);
			if (!overlapping)
				state = OUT_TOUCH;
			break;
		default:
			break;
	}
}


inline void Touchable::move()
{
	VECTOR2D c;
	GetARSG()->Convert3Dto2D(&c, GetPosition());
		
	//�g�̔���
	if (c.x < 0 || c.x > sizex)	vx *= -1.0f;
	if (c.y > sizey-50 && vy<0)	vy *= -1.0f;

	//���R�����܂��͒�~
	if (c.y > sizey-50 && vy<0.03f) 
		vy = 0;
	else
		vy -= 0.03f;

	//��C��R
	vx *= 0.8f;
	vy *= 0.8f;

	
  // SetPosition(vx, vy, 0.0f, GL_RELATIVE);
}


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	WindowManager program(hInstance, &MainLoop);
#ifdef DEBUG
    MessageBox(NULL,L"OK?",TEXT(APPNAME), NULL);
#endif
    return 0;
}
