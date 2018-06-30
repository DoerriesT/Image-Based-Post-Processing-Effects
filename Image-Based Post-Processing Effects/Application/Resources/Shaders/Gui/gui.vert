#version 450 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;

out vec2 vTexCoord;
out vec2 vPosition;
out vec4 vColor;
flat out int vShouldBlur;

uniform mat4 uProjection;
uniform int uBlurOn = 0;

void main() 
{
   vTexCoord = aTexCoord;
   vColor = aColor;
   
   gl_Position = uProjection * vec4(aPosition.xy, 0, 1);
   vPosition = gl_Position.xy;
   
   if (uBlurOn == 0 || aColor.a == 1.0)
   {
      vShouldBlur = 0;
   }
   else
   {
      vShouldBlur = 1;
   }
}