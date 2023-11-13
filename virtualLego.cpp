////////////////////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: 박창현 Chang-hyeon Park, 
// Modified by Bong-Soo Sohn and Dong-Jun Kim
// 
// Originally programmed for Virtual LEGO. 
// Modified later to program for Virtual Billiard.
//        
////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>

IDirect3DDevice9* Device = NULL;

// window size
const int Width  = 1080;
const int Height = 760;

int score = 0;

// There are 39 balls
const int NUM_BALLS = 39;
//pressed space
bool pressed_space = false;
//pause flag
bool pause = false;
//Variable for Save the velocity of bullet ball
double temp_x, temp_z;
// initialize the position (coordinate) of each ball (ball0 ~ ball39)
const float spherePos[NUM_BALLS][2] = {{-2.9f,1.7},{-2.4f,1.7},{-1.9f,1.7}, {-3.3f,1.3},{-3.3f,0.7},{-3.3f,0.2},{-3.3f,-0.3}, {-1.5f,1.3},{-1.5f,0.7},{-1.5f,0.2},{-1.5f,-0.3}, {-2.9f,-0.7},{-2.4f,-0.7},{-1.9f,-0.7},
									   {-0.5f,1.7},{0.0f,1.7},{0.50f,1.7}, {-0.93f,1.3},{-0.93f,0.7},{-0.93f,0.2},{-0.93f,-0.3}, {0.95f,1.3},{0.95f,0.7},{0.95f,0.2},{0.95f,-0.3}, {-0.5f,-0.7},{0.0f,-0.7},{0.50f,-0.7},
									   {2.03f,1.7},{2.48f,1.7},{2.93f,1.7}, {1.61f,1.3},{1.61f,0.7},{1.61f,0.2},{1.61f,-0.3},{1.61f,-0.8}, {3.3f,1.3} ,{2.2f,0.7},{2.8f,0.7}};
// initialize the color of each ball (ball0 ~ ball39)
const D3DXCOLOR sphereColor[NUM_BALLS] = {};

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.21   // ball radius
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 0.9982

// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------

class CSphere {
private :
	float					center_x, center_y, center_z;
    float                   m_radius;
	float					m_velocity_x;
	float					m_velocity_z;
	bool					is_control_ball = false;

public:
    CSphere(void)
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_radius = 0;
		m_velocity_x = 0;
		m_velocity_z = 0;
        m_pSphereMesh = NULL;
    }
    ~CSphere(void) {}

public:
    bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;
		
        m_mtrl.Ambient  = color;
        m_mtrl.Diffuse  = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power    = 5.0f;
		
        if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))
            return false;
        return true;
    }
	
    void destroy(void)
    {
        if (m_pSphereMesh != NULL) {
            m_pSphereMesh->Release();
            m_pSphereMesh = NULL;
        }
    }

    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
		m_pSphereMesh->DrawSubset(0);
    }
	
    bool hasIntersected(CSphere& ball) 
	{
		// Insert your code here.
		float diff_x, diff_z;
		diff_x = this->getCenter().x - ball.getCenter().x;
		diff_z = this->getCenter().z - ball.getCenter().z;
		if(sqrt(pow(diff_x, 2) + pow(diff_z, 2)) <= ball.getRadius() + this->getRadius())
			return true;
		return false;
	}
	
	void hitBy(CSphere& ball) 
	{ 
		// Insert your code here.
		if (hasIntersected(ball)) {
			float delta_x = this->getCenter().x - ball.getCenter().x;
			float delta_z = this->getCenter().z - ball.getCenter().z;
			float speed = sqrt(pow(ball.getVelocity_X(), 2) + pow(ball.getVelocity_Z(), 2));
			float distance = sqrt(pow(delta_x, 2) + pow(delta_z, 2));
			float delta_time = distance / speed;
			ball.setPower(-delta_x / delta_time, -delta_z / delta_time);

			if (!is_control_ball) {
				this->setCenter(999.0f, 999.0f, -999.0f);
				score++;
			}

		}
	}

	void ballUpdate(float timeDiff) 
	{
		const float TIME_SCALE = 3.3;
		D3DXVECTOR3 cord = this->getCenter();
		double vx = abs(this->getVelocity_X());
		double vz = abs(this->getVelocity_Z());

		if(vx > 0.01 || vz > 0.01)
		{
			float tX = cord.x + TIME_SCALE*timeDiff*m_velocity_x;
			float tZ = cord.z + TIME_SCALE*timeDiff*m_velocity_z;

			//correction of position of ball
			// Please uncomment this part because this correction of ball position is necessary when a ball collides with a wall
			if(tX >= (3.8 - M_RADIUS))
				tX = 3.8 - M_RADIUS;
			else if(tX <=(-3.8 + M_RADIUS))
				tX = -3.8 + M_RADIUS;

			//else if(tZ <= (-3 + M_RADIUS))
			//	tZ = -3 + M_RADIUS;
			else if(tZ >= (3 - M_RADIUS))
				tZ = 3 - M_RADIUS;
			
			this->setCenter(tX, cord.y, tZ);
		}
		else { this->setPower(0,0);}
		
		
	}

	double getVelocity_X() { return this->m_velocity_x;	}
	double getVelocity_Z() { return this->m_velocity_z; }
	bool getIsControlBall() { return this->is_control_ball; }

	void setPower(double vx, double vz)
	{
		this->m_velocity_x = vx;
		this->m_velocity_z = vz;
	}

	void setCenter(float x, float y, float z)
	{
		D3DXMATRIX m;
		center_x=x;	center_y=y;	center_z=z;
		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	void setIsControlBall(bool is_control_ball)
	{
		this->is_control_ball = is_control_ball;
	}
	
	float getRadius(void)  const { return (float)(M_RADIUS);  }
    const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
    D3DXVECTOR3 getCenter(void) const
    {
        D3DXVECTOR3 org(center_x, center_y, center_z);
        return org;
    }
	
private:
    D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
    ID3DXMesh*              m_pSphereMesh;
	
};



// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall {

private:
	
    float					m_x;
	float					m_z;
	float                   m_width;
    float                   m_depth;
	float					m_height;
	
public:
    CWall(void)
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_width = 0;
        m_depth = 0;
        m_pBoundMesh = NULL;
    }
    ~CWall(void) {}
public:
    bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;
		
        m_mtrl.Ambient  = color;
        m_mtrl.Diffuse  = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power    = 5.0f;
		
        m_width = iwidth;
        m_depth = idepth;
		
        if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
            return false;
        return true;
    }
    void destroy(void)
    {
        if (m_pBoundMesh != NULL) {
            m_pBoundMesh->Release();
            m_pBoundMesh = NULL;
        }
    }
    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
		m_pBoundMesh->DrawSubset(0);
    }
	
	bool hasIntersected(CSphere& ball) {
		// Insert your code here.
		float l_limit = this->m_x - this->m_width / 2 - M_RADIUS;
		float r_limit = this->m_x + this->m_width / 2 + M_RADIUS;
		float t_limit = this->m_z - this->m_depth / 2 - M_RADIUS;

		if (ball.getCenter().x == l_limit || ball.getCenter().x == r_limit || ball.getCenter().z == t_limit) {
			return true;
		}

		return false;
	}

	void hitBy(CSphere& ball) {
		// Insert your code here.
		if (hasIntersected(ball)) {
			float l_limit = this->m_x - this->m_width / 2 - M_RADIUS;
			float r_limit = this->m_x + this->m_width / 2 + M_RADIUS;
			float t_limit = this->m_z - this->m_depth / 2 - M_RADIUS;
			if (ball.getCenter().z == t_limit) {
				ball.setPower(ball.getVelocity_X(), -ball.getVelocity_Z());
			}
			if (ball.getCenter().x == l_limit) {
				ball.setPower(-ball.getVelocity_X(), ball.getVelocity_Z());
			}
			if (ball.getCenter().x == r_limit) {
				ball.setPower(-ball.getVelocity_X(), ball.getVelocity_Z());
			}
		}
	}    
	
	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		this->m_x = x;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}
	
    float getHeight(void) const { return M_HEIGHT; }
	
	
	
private :
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	
	D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
    ID3DXMesh*              m_pBoundMesh;
};

// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight {
public:
    CLight(void)
    {
        static DWORD i = 0;
        m_index = i++;
        D3DXMatrixIdentity(&m_mLocal);
        ::ZeroMemory(&m_lit, sizeof(m_lit));
        m_pMesh = NULL;
        m_bound._center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        m_bound._radius = 0.0f;
    }
    ~CLight(void) {}
public:
    bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f)
    {
        if (NULL == pDevice)
            return false;
        if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL)))
            return false;
		
        m_bound._center = lit.Position;
        m_bound._radius = radius;
		
        m_lit.Type          = lit.Type;
        m_lit.Diffuse       = lit.Diffuse;
        m_lit.Specular      = lit.Specular;
        m_lit.Ambient       = lit.Ambient;
        m_lit.Position      = lit.Position;
        m_lit.Direction     = lit.Direction;
        m_lit.Range         = lit.Range;
        m_lit.Falloff       = lit.Falloff;
        m_lit.Attenuation0  = lit.Attenuation0;
        m_lit.Attenuation1  = lit.Attenuation1;
        m_lit.Attenuation2  = lit.Attenuation2;
        m_lit.Theta         = lit.Theta;
        m_lit.Phi           = lit.Phi;
        return true;
    }
    void destroy(void)
    {
        if (m_pMesh != NULL) {
            m_pMesh->Release();
            m_pMesh = NULL;
        }
    }
    bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return false;
		
        D3DXVECTOR3 pos(m_bound._center);
        D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
        D3DXVec3TransformCoord(&pos, &pos, &mWorld);
        m_lit.Position = pos;
		
        pDevice->SetLight(m_index, &m_lit);
        pDevice->LightEnable(m_index, TRUE);
        return true;
    }

    void draw(IDirect3DDevice9* pDevice)
    {
        if (NULL == pDevice)
            return;
        D3DXMATRIX m;
        D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
        pDevice->SetTransform(D3DTS_WORLD, &m);
        pDevice->SetMaterial(&d3d::WHITE_MTRL);
        m_pMesh->DrawSubset(0);
    }

    D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }

private:
    DWORD               m_index;
    D3DXMATRIX          m_mLocal;
    D3DLIGHT9           m_lit;
    ID3DXMesh*          m_pMesh;
    d3d::BoundingSphere m_bound;
};

// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall	g_legoPlane;
CWall	g_legowall[3];
CSphere	g_sphere[NUM_BALLS];
CSphere	g_ControlBall;
CSphere g_BulletBall;
CLight	g_light;

//double g_camera_pos[3] = {0.0, 5.0, -8.0};
bool game_over = false;

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------


void destroyAllLegoBlock(void)
{
}

// initialization
bool Setup()
{
	int i;
	
    D3DXMatrixIdentity(&g_mWorld);
    D3DXMatrixIdentity(&g_mView);
    D3DXMatrixIdentity(&g_mProj);
		
	// create plane and set the position
    if (false == g_legoPlane.create(Device, -1, -1, 9, 0.03f, 6, d3d::GREEN)) return false;
    g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);

	
	// create walls and set the position. note that there are four walls
	if (false == g_legowall[0].create(Device, -1, -1, 7.6, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[0].setPosition(0.0f, 0.12f, 3.06f);
	if (false == g_legowall[1].create(Device, -1, -1, 0.12f, 0.3f, 6.12f, d3d::DARKRED)) return false;
	g_legowall[1].setPosition(3.86f, 0.12f, 0.06f);
	if (false == g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 6.12f, d3d::DARKRED)) return false;
	g_legowall[2].setPosition(-3.86f, 0.12f, 0.06f);

	// create four balls and set the position
	for (i=0;i< NUM_BALLS;i++) {
		if (false == g_sphere[i].create(Device, sphereColor[i])) return false;
		g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS , spherePos[i][1]);
		g_sphere[i].setPower(0,0);
	}
	
	// create a control ball and set the position
	if (false == g_ControlBall.create(Device, d3d::WHITE)) return false;
	g_ControlBall.setCenter(0.0f, (float)M_RADIUS, -3.0f);
	g_ControlBall.setIsControlBall(true);

	// create a bullet ball and set the position
	if (false == g_BulletBall.create(Device, d3d::RED)) return false;
	g_BulletBall.setCenter(0.0f, (float)M_RADIUS, -3.21f + 2 * M_RADIUS);
	
	// light setting 
    D3DLIGHT9 lit;
    ::ZeroMemory(&lit, sizeof(lit));
    lit.Type         = D3DLIGHT_POINT;
    lit.Diffuse      = d3d::WHITE; 
	lit.Specular     = d3d::WHITE * 0.9f;
    lit.Ambient      = d3d::WHITE * 0.9f;
    lit.Position     = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
    lit.Range        = 100.0f;
    lit.Attenuation0 = 0.0f;
    lit.Attenuation1 = 0.9f;
    lit.Attenuation2 = 0.0f;
    if (false == g_light.create(Device, lit))
        return false;
	
	// Position and aim the camera.
	D3DXVECTOR3 pos(0.0f, 7.0f, -5.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &g_mView);
	
	// Set the projection matrix.
	D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4,
        (float)Width / (float)Height, 1.0f, 100.0f);
	Device->SetTransform(D3DTS_PROJECTION, &g_mProj);
	
    // Set render states.
    Device->SetRenderState(D3DRS_LIGHTING, TRUE);
    Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
    Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	
	g_light.setLight(Device, g_mWorld);
	return true;
}

void Cleanup(void)
{
    g_legoPlane.destroy();
	for(int i = 0 ; i < 3; i++) {
		g_legowall[i].destroy();
	}
    destroyAllLegoBlock();
    g_light.destroy();
}


// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta)
{
	int i=0;
	int j = 0;


	if( Device )
	{
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
		Device->BeginScene();

		//initial Ball setting`
		if (!pressed_space) {
			g_BulletBall.setCenter(g_ControlBall.getCenter().x, g_ControlBall.getCenter().y, g_ControlBall.getCenter().z + 2 * M_RADIUS);
		}
		
		// update the position of each ball. during update, check whether each ball hit by walls.
		for( i = 0; i < NUM_BALLS; i++) {
			g_sphere[i].ballUpdate(timeDelta);
			for (j = 0; j < 3; j++) { g_legowall[i].hitBy(g_BulletBall); }
		}
		g_BulletBall.ballUpdate(timeDelta);

		// check whether any two balls hit together and update the direction of balls
		for(i = 0 ;i < NUM_BALLS; i++){
			g_sphere[i].hitBy(g_BulletBall);
		}

		g_ControlBall.hitBy(g_BulletBall);

		if (g_BulletBall.getCenter().z <= -3.5f) {
			game_over = true;
			g_BulletBall.setPower(0, 0);
		}

		// draw plane, walls, and spheres
		g_legoPlane.draw(Device, g_mWorld);
		for (i=0;i<3;i++) 	{
			g_legowall[i].draw(Device, g_mWorld);
		}
		for (i = 0; i < NUM_BALLS; i++) {
			g_sphere[i].draw(Device, g_mWorld);
		}
		g_ControlBall.draw(Device, g_mWorld);
		g_BulletBall.draw(Device, g_mWorld);
        g_light.draw(Device);
		
		Device->EndScene();
		Device->Present(0, 0, 0, 0);
		Device->SetTexture( 0, NULL );
	}
	return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool wire = false;
	static bool isReset = true;
    static int old_x = 0;
    static int old_y = 0;
    static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;
	static float l_limit = -3.86f;
	static float r_limit = 3.86f;

	switch (msg) {
	case WM_DESTROY:
	{
		::PostQuitMessage(0);
		break;
	}
	case WM_KEYDOWN:
	{

		switch (wParam) {
		case VK_ESCAPE:
			::DestroyWindow(hwnd);
			break;
		case VK_RETURN:
			if (NULL != Device) {
				wire = !wire;
				Device->SetRenderState(D3DRS_FILLMODE,
					(wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
			}
			break;
		case VK_SPACE:
			if (!pressed_space) {
				pressed_space = true;
				g_BulletBall.setPower(0, 2.0f);
			}
			break;
		//restart the game when you pressed 'r'
		case 0x52:
			if (game_over) {
				score = 0;
				game_over = false;
				pressed_space = false;
				for (int i = 0; i < NUM_BALLS; i++) {
					g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
					g_sphere[i].setPower(0, 0);
				}
				g_ControlBall.setCenter(0.0f, (float)M_RADIUS, -3.0f);
				g_BulletBall.setCenter(0.0f, (float)M_RADIUS, -3.21f + 2 * M_RADIUS);
				g_BulletBall.setPower(0, 0);
			}
			break;
		//pause the game when you pressed 'p'
		case 0x50:
			if (!game_over) {
				pause = !pause;
				if (pause) {
					temp_x = g_BulletBall.getVelocity_X();
					temp_z = g_BulletBall.getVelocity_Z();
					g_BulletBall.setPower(0, 0);
				}
				else {
					g_BulletBall.setPower(temp_x, temp_z);
				}
			}
			break;
		default:
			break;
		}
		break;
	}
	case WM_MOUSEMOVE:
	{
		//마우스가 따라가는 곳으로 ControlBall이 이동하게끔
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		//LOWORD의 이동만 따라감
		float normalizedX = static_cast<float>(x) / static_cast<float>(Width) * 2.0f - 1.0f;
		if(!pause && !game_over)g_ControlBall.setCenter(normalizedX * 3.8f, g_ControlBall.getCenter().y, g_ControlBall.getCenter().z);

		break;
	}

	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{
    srand(static_cast<unsigned int>(time(NULL)));
	
	if(!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
	
	if(!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}
	
	d3d::EnterMsgLoop( Display );
	
	Cleanup();
	
	Device->Release();
	
	return 0;
}