#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D currentState;    // current grid
uniform ivec2 gridSize;			   // (width, height)

int GetCellState(ivec2 pos) {
	//pos = ivec2(mod(vec2(pos), vec2(gridSize))); // wrapping
	//float state = texture(currentState, vec2(pos) / vec2(gridSize)).r;
	float state = texelFetch(currentState,pos, 0).r;
	return state > .5 ? 1 : 0;
}

void main()
{
	vec3 _color = vec3(0.361, 0.89, 0.82);
    
	// vec2 pos = TexCoord * vec2(gridSize);

	// vec3 finalColor;
	// if ((pos.x + pos.y) % 2 == 0) {
    //     // Main color
    //     finalColor = texelColor.rgb;
    // } else {
    //     // Background color
    //     finalColor = backgroundColor;
    // }

	// //int cellState = GetCellState(pos);

	float s = texture(currentState, TexCoord).r;

    FragColor = vec4(s * _color, 1.0f);
}