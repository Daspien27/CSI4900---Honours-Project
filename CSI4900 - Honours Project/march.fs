#version 330 core

in vec4 colorVertFrag;
in vec4 pos;

out vec4 color;

void main() {
  // color the fragment
  //color = vec4((sin(pos.x)+1)/2.0, (sin(pos.y)+1)/2.0, (sin(pos.z)+1)/2.0,1.0);
  color = pos;
  //color = colorVertFrag;
}
