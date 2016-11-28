#version 330 core

in vec4 posFrag;
in vec4 colourVertFrag;
in vec3 normalFrag; 
in vec3 eyeFrag; 
in vec3 lightFrag; 

struct LightSource {
  vec4 ambient;  
  vec4 diffuse;
  vec4 specular;
 
  vec3 spot_direction;
  float spot_exponent;
  float spot_cutoff;
 
  float constant_attenuation;
  float linear_attenuation;
  float quadratic_attenuation;
};

uniform LightSource lights[1];
uniform vec4 lightPosition[1];

out vec4 colour;

void main() {
  vec3 NVec = normalize(normalFrag);
  vec3 LVec = normalize(lightFrag);
  vec3 EVec = normalize(eyeFrag);

  float distanceLight = length(lightFrag.xyz);

  float attenuation = 1.0 / 
    (lights[0].constant_attenuation +
     lights[0].linear_attenuation * distanceLight +
     lights[0].quadratic_attenuation * distanceLight * distanceLight);

  // ambient term
  vec4 ambient =  colourVertFrag * lights[0].ambient;

  // diffuse term
  float dotNL = max(0.0,dot(NVec,LVec)); //max(-dot(NVec,LVec),dot(NVec,LVec));
  vec4 diffuse = colourVertFrag * lights[0].diffuse * dotNL;

  // spot light
  float spot_attenuation = 1.0;
  float dotSV = dot(-LVec,normalize(lights[0].spot_direction));
  if ( dotSV < cos(radians(lights[0].spot_cutoff))) {
    spot_attenuation = 0.0;
  } else {
    spot_attenuation = pow(dotSV,lights[0].spot_exponent);
  }

	// colour
	colour = attenuation * spot_attenuation * diffuse * colourVertFrag;
	colour += ambient;

}
