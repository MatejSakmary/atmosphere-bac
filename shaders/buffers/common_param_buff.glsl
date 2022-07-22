layout(set = 0, binding = 0) uniform CommonParamBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    float time;
	mat4 lHviewProj;
} commonParameters;