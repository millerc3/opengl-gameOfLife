#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoord;

uniform sampler2D currentState;    // current grid
uniform ivec2 gridSize;			   // (width, height)

int GetCellState(ivec2 pos) {
	pos = ivec2(mod(vec2(pos), vec2(gridSize))); // wrapping
	float state = texture(currentState, vec2(pos) / vec2(gridSize)).r;
	return state > .5 ? 1 : 0;
}

int GetLiveNeighborCount(ivec2 pos) {
	int liveNeighbors = 0;

	for (int xOffset = -1; xOffset < 2; xOffset++) {
		for (int yOffset = -1; yOffset < 2; yOffset++) {
			int neighborState = GetCellState(pos + ivec2(xOffset, yOffset));
			liveNeighbors += neighborState;
		}
	}
	return liveNeighbors;
}

void main()
{
	ivec2 pos = ivec2(TexCoord * vec2(gridSize));

	int cellState = GetCellState(pos);
	int newState = cellState;

	int neighborCount = GetLiveNeighborCount(pos);
	if (cellState == 1) {
		if (neighborCount < 2 || neighborCount > 4) {
			newState = 0;
		}
	}
	else {
		if (neighborCount == 3) {
			newState = 1;
		}
	}

	newState = 1;

	FragColor = vec4(1); //vec4(newState * vec3(0.361, 0.89, 0.82), 1.0f);
}