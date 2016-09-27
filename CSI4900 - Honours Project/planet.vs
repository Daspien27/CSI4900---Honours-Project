#version 330 core

layout (location=0) in vec4 position;

out vec4 colorVertFrag; // Pass the color on to rasterization
out vec4 pos;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main() {
  const vec4 vertColor = vec4( 1.0, 1.0, 0.0, 0.0 );	
   
  //A more uniform sphere
  
  gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * position;
  colorVertFrag = vertColor;
  pos = ModelMatrix * position;
}

