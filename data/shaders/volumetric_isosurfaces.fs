
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform mat4 u_inv_model;
uniform vec3 u_camera_position;

uniform sampler3D u_texture;
uniform sampler2D u_noise_texture;
uniform sampler2D u_transfer_function_texture;

uniform vec3 rayDirection;
uniform vec3 samplePos;

uniform vec4 sample_color;
uniform vec4 u_color;
uniform vec4 u_clipping_plane;

uniform float stepLength;
uniform float u_brightness;
uniform float alpha_color;
uniform float u_texture_width;
uniform float u_threshold;

uniform bool u_jittering;
uniform bool u_jittering2;
uniform bool u_clipping_check;
uniform bool u_check_transfer_function;

// Phong
uniform vec3 u_light_position;

uniform mat4 u_model;
uniform float u_h;
uniform bool u_normals;

const int MAX_ITERATION = 1000;

float rand(vec2 co){
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
	//1. Setup ray
	//Init variables
	vec3 camera_localpos = (u_inv_model * vec4(u_camera_position, 1.0)).xyz;
	vec3 rayDirection = normalize(v_position - camera_localpos);					//Ray direction
	vec3 stepRay = rayDirection * stepLength;										//Step ray

	vec3 samplePos = v_position;
	vec4 finalColor = vec4(0.0);

	// Jittering 1st approach
	if (u_jittering){
		vec2 uv_noise = vec2(gl_FragCoord.xy / u_texture_width);
		vec3 noise_value = texture2D(u_noise_texture, uv_noise).xyz;
		samplePos += stepRay * noise_value;
	}

	// Jittering 2nd approach
	if (u_jittering2){
		samplePos += stepRay * rand(gl_FragCoord.xy);
	}

	//For loop
	for (int i = 0; i < MAX_ITERATION; i++)
	{
		// Clipping
		bool clipping = true;
		if (u_clipping_check){
			clipping = (u_clipping_plane.x * samplePos.x + u_clipping_plane.y * samplePos.y + 
			u_clipping_plane.z * samplePos.z + u_clipping_plane.w <= 0);
		}
		if (clipping){
			
			// 2. Get information from 3D texture
			vec3 textCoords = (samplePos + 1.0) / 2.0;												//Volume sampling
			float d = texture3D(u_texture, textCoords).x;											//Density of sample position
			
			if (d > u_threshold){

				// Gradient computation
				vec3 gradient = vec3(0.0);

				vec3 textCoord1x = (vec3(samplePos.x + u_h, samplePos.y, samplePos.z) + 1.0) / 2.0;	//Local to texture coordinates space
				vec3 textCoord2x = (vec3(samplePos.x - u_h, samplePos.y, samplePos.z) + 1.0) / 2.0;
				float fx1 = texture3D(u_texture, textCoord1x).x;									//Read the texture
				float fx2 = texture3D(u_texture, textCoord2x).x;

				vec3 textCoord1y = (vec3(samplePos.x, samplePos.y + u_h, samplePos.z) + 1.0) / 2.0;
				vec3 textCoord2y = (vec3(samplePos.x, samplePos.y - u_h, samplePos.z) + 1.0) / 2.0;
				float fy1 = texture3D(u_texture, textCoord1y).x;
				float fy2 = texture3D(u_texture, textCoord2y).x;

				vec3 textCoord1z = (vec3(samplePos.x, samplePos.y, samplePos.z + u_h) + 1.0) / 2.0;
				vec3 textCoord2z = (vec3(samplePos.x, samplePos.y, samplePos.z - u_h) + 1.0) / 2.0;
				float fz1 = texture3D(u_texture, textCoord1z).x;
				float fz2 = texture3D(u_texture, textCoord2z).x;

				gradient.x = -(1.0/ (2.0 * u_h) * (fx1 - fx2));										//Negative gradient corresponds to the normal
				gradient.y = -(1.0/ (2.0 * u_h) * (fy1 - fy2));
				gradient.z = -(1.0/ (2.0 * u_h) * (fz1 - fz2));
				gradient = normalize(gradient);

				if (u_normals) {finalColor = vec4(gradient, 1.0); break;}							//Show normals

				// 3. Obtain color from density obtained
				vec4 sampleColor;

				// Transfer function
				if (u_check_transfer_function){
					sampleColor = texture2D(u_transfer_function_texture, vec2(d, 0.5));
					sampleColor.xyz *= sampleColor.w;
				}else{
					sampleColor = vec4(d, d, d, d);													//Color of sample position
				}

				//Phong equation
				if (sampleColor.w == 1.0){ //not transparent

					vec3 light_localpos = (u_inv_model * vec4(u_light_position, 1.0)).xyz;

					vec3 L = normalize(light_localpos - samplePos);
					float LdotN = dot(L, gradient);

					vec3 R = reflect(-L, gradient);
					vec3 V = normalize(camera_localpos - samplePos);
					float RdotV = pow(dot(R, V), 10.0);

					vec4 light_equation = vec4(sampleColor.xyz * (LdotN + RdotV), 1.0);
					finalColor = light_equation;
					break;
				}

				//4. Composition of final_color
				finalColor += stepLength * (1.0 - finalColor.a) * sampleColor;						//Final color
			}
		}

		//5. Next Sample
		samplePos += stepRay;

		//6. Early termination
		if (finalColor.w >= 1.0 ||																	//Sample outisde the volume
		(samplePos.x < -1.0 || samplePos.x > 1.0) || 
		(samplePos.y < -1.0 || samplePos.y > 1.0) || 
		(samplePos.z < -1.0 || samplePos.z > 1.0))
			break;
	}

	if (finalColor.w < alpha_color){																// Very low color alpha
		discard;
	}

	gl_FragColor = finalColor * u_brightness * u_color;
}
