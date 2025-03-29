#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoord;

uniform sampler2D currentState;    // current grid
uniform ivec2 gridSize;			   // (width, height)

int GetCellState(ivec2 pos) {
	float state = texture(currentState, vec2(pos) / vec2(gridSize)).r;
	return state > .5 ? 1 : 0;
}

int GetLiveNeighborCount(ivec2 pos) {
	int liveNeighbors = 0;

	for (int xOffset = -1; xOffset < 2; xOffset++) {
		for (int yOffset = -1; yOffset < 2; yOffset++) {
			if (xOffset == 0 || yOffset == 0) continue;
			int neighborState = GetCellState(pos + ivec2(xOffset, yOffset));
			liveNeighbors += neighborState;
		}
	}
	return liveNeighbors;
}

void main()
{
	ivec2 pos = ivec2(TexCoord.x * gridSize.x, TexCoord.y * gridSize.y);

	if (pos.x == 0 || pos.y == 0 || pos.x == gridSize.x - 1|| pos.y == gridSize.y - 1) {
		FragColor = vec4(vec3(0), 1.0f);
		return;
	}

	int cellState = GetCellState(pos);
	int newState = cellState;

	int neighborCount = GetLiveNeighborCount(pos);
	if (cellState == 1) {
		if (neighborCount < 2 || neighborCount > 3) {
			newState = 0;
		}
	}
	else {
		if (neighborCount == 3) {
			newState = 1;
		}
	}

    FragColor = vec4(vec3(newState), 1.0f);
}