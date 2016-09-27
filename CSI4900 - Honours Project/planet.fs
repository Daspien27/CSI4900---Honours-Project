#version 330 core

in vec4 colorVertFrag;
in vec4 pos;

out vec4 color;

void main() {
  // color the fragment 
  color = colorVertFrag / (14/length(pos));
}
