#version 330 core

in vec4 colorVertFrag;
in vec4 pos;

out vec4 color;

void main() {
  // color the fragment 
  
  // If the points are on the sphere, black, and white if not

  if(dot(pos,pos) == 49.0f){
	color = vec4(0,0,0,1);
  }else{
	color = vec4(1,1,1,1);
  }


  //color = vec4((sin(pos.x)+1)/2.0, (sin(pos.y)+1)/2.0, (sin(pos.z)+1)/2.0,0.5);
}
