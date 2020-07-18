#include "GLTools.h"
#include "GLShaderManager.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"
#include "StopWatch.h"

#include <math.h>
#include <stdio.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

//001--综合训练(地板)
GLShaderManager		shaderManager;			// 着色器管理器
GLMatrixStack		modelViewMatrix;		// 模型视图矩阵
GLMatrixStack		projectionMatrix;		// 投影矩阵
GLFrustum			viewFrustum;			// 视景体
GLGeometryTransform	transformPipeline;		// 几何图形变换管道

GLTriangleBatch		torusBatch;             //大球
GLTriangleBatch     sphereBatch;            //小球
GLBatch                floorBatch;          //地板

//角色帧 照相机角色帧
GLFrame             cameraFrame;

//**4、添加附加随机球
#define NUM_SPHERES 50
GLFrame spheres[NUM_SPHERES];

void SetupRC()
{
   
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //初始化
    
    shaderManager.InitializeStockShaders();
    //深度测试
    glEnable(GL_DEPTH_TEST);
    
    //设置地步顶点数
    
    floorBatch.Begin(GL_LINES, 324);
    
    for(GLfloat x = -20.0; x <= 20.0f; x+= 0.5) {
           floorBatch.Vertex3f(x, -0.55f, 20.0f);
           floorBatch.Vertex3f(x, -0.55f, -20.0f);
           
           floorBatch.Vertex3f(20.0f, -0.55f, x);
           floorBatch.Vertex3f(-20.0f, -0.55f, x);
       }
    floorBatch.End();
    
   
    gltMakeSphere(torusBatch, 0.4f, 40, 80);
    
    //小球模型
    gltMakeSphere(sphereBatch, 0.1f, 13, 26);
    
    //随机50个
    for (int i = 0; i < NUM_SPHERES; i++) {
        
        //y轴不变，X,Z产生随机值
        GLfloat x = ((GLfloat)((rand() % 400) - 200 ) * 0.1f);
        GLfloat z = ((GLfloat)((rand() % 400) - 200 ) * 0.1f);
        
        //在y方向，将球体设置为0.0的位置，这使得它们看起来是飘浮在眼睛的高度
        //对spheres数组中的每一个顶点，设置顶点数据
        spheres[i].SetOrigin(x, 0.0f, z);
    }
    
}

//进行调用以绘制场景
void RenderScene(void)
{
    //设置求颜色
    
    //地板线颜色
    static GLfloat VFloorColor[] = {1.0f,1.0f,0.0f,1.0f};
    //球颜色
    static GLfloat VTorusColor[] = {1.0f,0.0f,1.0f,1.0f};
    //小球颜色
    static GLfloat vSphereColor[] = { 0.8f, 0.2f, 1.0f, 1.0f};
    
    
    //开启一个动画 获取角度
    static CStopWatch rotTimer;
    float yRot = rotTimer.GetElapsedSeconds() * 60.0f;
    
    //清除缓冲区
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    
    
     modelViewMatrix.PushMatrix(mCamera);
    
       //获取光源位置
       M3DVector4d vLightpos = {0.0f,10.0f,5.0f,1.0f};
    //绘制 平面着色器
    shaderManager.UseStockShader(GLT_SHADER_FLAT,transformPipeline.GetModelViewProjectionMatrix(),VFloorColor);
    
    floorBatch.Draw();
    
    modelViewMatrix.PushMatrix();
 
    //获取光源位置
    
    //  放这里 小球旋转没出来
 //  modelViewMatrix.PushMatrix();
    
    //大球移动
    modelViewMatrix.Translate(0.0f, 0.0f, -3.0f);
    
     //这里注意 移动只有一次就行 和旋转是分开的
    modelViewMatrix.PushMatrix();
    
    //大球自转
    
    modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
    
    //点光源
    shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(),vLightpos,VTorusColor);
    
    
    torusBatch.Draw();
    
    modelViewMatrix.PopMatrix();
    
    
    //小球渲染
    for (int i = 0; i < NUM_SPHERES; i++) {
           modelViewMatrix.PushMatrix();
           modelViewMatrix.MultMatrix(spheres[i]);
           shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(),
                                        transformPipeline.GetProjectionMatrix(), vLightpos, vSphereColor);
           sphereBatch.Draw();
           modelViewMatrix.PopMatrix();
           
       }
    
    
//    modelViewMatrix.PushMatrix();
    modelViewMatrix.Rotate(yRot * -2.0f, 0.0f, 1.0f, 0.0f);
    modelViewMatrix.Translate(0.8, 0.0f, 0.0f);
    shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(),vLightpos,vSphereColor);
    sphereBatch.Draw();
//    modelViewMatrix.PopMatrix();
    
    modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();
    
    glutSwapBuffers();
    glutPostRedisplay();
    
  
}

void SpeacialKeys(int key,int x,int y){
    //移动步长
    float linear = 0.1f;
    //旋转度数
    float angular = float(m3dDegToRad(5.0f));
    
    if (key == GLUT_KEY_UP) {
        //MoveForward 平移
        cameraFrame.MoveForward(linear);
    }
    if (key == GLUT_KEY_DOWN) {
        cameraFrame.MoveForward(-linear);
    }
    
    if (key == GLUT_KEY_LEFT) {
        //RotateWorld 旋转
        cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);
    }
    
    if (key == GLUT_KEY_RIGHT) {
        cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);
    }
    

}

//屏幕更改大小或已初始化
void ChangeSize(int nWidth, int nHeight)
{
 
    glViewport(0, 0, nWidth, nHeight);
    
    //投影
    viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
    //获取加入投影矩阵
    
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    //模型矩阵push一个单元矩阵
    //交给管线
    
    
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    
}





int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800 ,600);
    
    glutCreateWindow("OpenGL SphereWorld");
    
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
     glutSpecialFunc(SpeacialKeys);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    
    SetupRC();
    glutMainLoop();    
    return 0;
}
