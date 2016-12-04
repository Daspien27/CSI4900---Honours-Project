#version 330 core

layout (location=0) in vec4 position;
layout (location=1) in vec4 normal;
layout (location=2) in vec4 colour;


out vec4 posFrag;
out vec4 colourVertFrag; // Pass the color on to rasterization
out vec3 normalFrag;		// Pass the normal to rasterization
out vec3 eyeFrag;			// Pass an eye vector along
out vec3 lightFrag;			// Pass a light vector along

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform vec4 lightPosition[1];


void main() {
  const vec4 vertColor = vec4( 1.0, 1.0, 0.0, 0.0 );	
  
  //More code from A3 in CSI4130 
   vec4 posVec = ModelMatrix * ViewMatrix * position;
  eyeFrag = -posVec.xyz;  

  // light vector in camera coordinates
  // Check for directional lighting	
  
  if ( lightPosition[0].w > 0.0 ) {
    lightFrag = lightPosition[0].xyz - posVec.xyz;
  } else {
    lightFrag = lightPosition[0].xyz;
  }
  
  //lightFrag = vec3(ViewMatrix * vec4(lightFrag,1));
  
  // assume Modelview matrix has no non-uniform scaling or shearing 
  normalFrag = vec3(ViewMatrix * ModelMatrix * normal);
  //normalFrag = normal.xyz;
  posFrag = ModelMatrix * position;

  gl_Position = ProjectionMatrix * ViewMatrix * posFrag;
  colourVertFrag = colour;
 
}

