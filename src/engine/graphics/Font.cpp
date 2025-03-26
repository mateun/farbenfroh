//
// Created by mgrus on 25.03.2025.
//

#include "Font.h"

#include "Geometry.h"
#include "MeshDrawData.h"
#include "Renderer.h"

FBFont::FBFont(const std::string &fileName) {
    bitmap = std::make_unique<Bitmap>("../assets/font.bmp");

    // TODO handle the size? What if this is not a good size?
    texture  = Texture::createEmptyTexture(512, 32);
	quadMesh = gru::Geometry::createQuadMesh(PlanePivot::bottomleft);

}

void FBFont::renderText(const std::string &text, glm::vec3 position) {
    char* buf = (char*) malloc(text.length()+1);
    sprintf_s(buf, text.length()+1, text.c_str());
    //font(bitmap.get());
    updateAndDrawText(buf, position.x,position.y, position.z);
    free(buf);
}


void FBFont::clearText() {
    uint64_t *bb64 = (uint64_t *) texture->pixels();
    for (int i = 0; i < (texture->width() * texture->height()) / 2; i++) {
        *bb64 = 0;
        *bb64++;
    }
}

/**
 * Adds text to an existing texture. E.g. for multiline texts.
 * @param text The text to add.
 * @param tex The texture to modify.
 * @param x The x-position of the text in the bitmap.
 * @param y The y-position of the text in the bitmap.
 */
void FBFont::addText(const std::string& text, int x, int y) {
    //renderTargetBitmap(tex->bitmap);
    drawText(text.c_str(), x, texture->height()-y);
    updateTextTexture(texture->width(), texture->height());
}

void FBFont::updateTextTexture(int w, int h) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->handle());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels());
	glBindTexture(GL_TEXTURE_2D, 0);
}


void FBFont::updateAndDrawText(const char *text, int screenPosX, int screenPosY, int screenPosZ) {

    auto bb64 = reinterpret_cast<uint64_t *>(texture->pixels());
    for (int i = 0; i < (texture->width() * texture->height()) / 2; i++) {
        *bb64 = 0;
        *bb64++;
    }

    drawText(text, 0, texture->height()-8);
    updateTextTexture(texture->width(), texture->height());

	glm::vec2 textScale = {10, 10};

	MeshDrawData mdd;
    mdd.scale = {static_cast<float>(texture->width()) * textScale.x,
           static_cast<float>(texture->height()) * textScale.y,
           1};

    mdd.location = glm::vec3{static_cast<float>(screenPosX), static_cast<float>(screenPosY), screenPosZ};
    mdd.texture = texture.get();
	mdd.mesh = quadMesh.get();
    Renderer::drawMesh(mdd);


}


void FBFont::drawText(const char* text, int posx, int posy) {

    auto lastC = ' ';
    int spacing = 5;
	for (int i = 0; i< strlen(text); i++) {


		auto c = text[i];

		if (c >= 65 && c <= 90) {
			drawBitmapTile(posx, posy,8, c-65, 0, bitmap.get());

		}
        else if (c == 'p') {
			drawBitmapTile(posx, posy,8, c-97, 1, bitmap.get(), 0, -1);
		}
		else if (c == 'g' || c == 'j') {
			drawBitmapTile(posx, posy,8, c-97, 1, bitmap.get(), 0, -1);
		}
		else if (c >= 97 && c <= 122) {
            lastC = c;
			drawBitmapTile(posx, posy,8, c-97, 1, bitmap.get());
		} else if (c == 32) {
			spacing = 4;
		} else if (c >= 48 && c <= 57) {
            lastC = c;
			drawBitmapTile(posx, posy,8, c-48, 2, bitmap.get(), 0, -1);
		} else if (c == ':') {
            spacing = 4;
			drawBitmapTile(posx, posy,8, 0, 3, bitmap.get(), 0, -1);
		} else if (c == '(') {
            spacing = 0;
            drawBitmapTile(posx, posy,8, 8, 3, bitmap.get());
        } else if (c == '/') {
            spacing = 3;
            drawBitmapTile(posx, posy,8, 4, 3, bitmap.get());
        } else if (c == '\\') {
            spacing = 3;
            drawBitmapTile(posx, posy,8, 5, 3, bitmap.get());
        } else if (c == ')') {
            spacing = 0;
            drawBitmapTile(posx, posy,8, 9, 3, bitmap.get());
        } else if (c == '.') {
            spacing = 4;
			drawBitmapTile(posx, posy,8, 10, 3, bitmap.get(), 0, 0);
		} else if (c == '-') {
            spacing = 3;
            drawBitmapTile(posx, posy,8, 3, 3, bitmap.get());
        }
        if (lastC == 'l' || lastC == 'f' || lastC == 'r') {
            spacing = 4;
        } else if (lastC == 'i') {
            spacing = 6;
        }

        else {
            spacing = 7;
        }
        lastC = ' ';

        posx += spacing;
	}

}

void FBFont::drawBitmapTile(int posx, int posy, int tileSize, int tilex, int tiley, Bitmap* bitmap, int offsetX, int offsetY) {
	int srcIndex = 0;

	for (int y = 0; y < tileSize; y++) {
		for (int x = 0; x < tileSize; x++) {
			srcIndex = (((tilex * tileSize) + x) * 4) + (((bitmap->height - tileSize - (tileSize* tiley)) + y)*4 * bitmap->width);
			setPixel(x+posx+offsetX, y+posy+offsetY,
					bitmap->pixels[srcIndex+2] ,
					bitmap->pixels[srcIndex+1],
					bitmap->pixels[srcIndex+0],
					bitmap->pixels[srcIndex+3]);
		}
	}


}


void FBFont::setPixel(int x, int y, int r, int g, int b, int a) {

	auto bb = texture->pixels();
	int pixel = (x*4) + (y* texture->width() * 4);

	// TODO: this actually breaks text rendering.
	// It works if blending is turned off:
	// tf = 1; why?
	float tf = ((float)a/255.0f);
	//tf = 1;

	if (pixel > texture->width() * texture->height() * 4) {
		printf("overflow!\n");
		exit(2);
	}
	glm::vec4 col = {1, 1, 1, 1};
	bb[pixel] = r * col.x * tf + (bb[pixel]*(1-tf));
	bb[pixel+1] = g* col.y * tf + (bb[pixel+1]* (1-tf));
	bb[pixel+2] = b*col.z * tf + (bb[pixel+2]* (1-tf));
	bb[pixel+3] = a* col.w;
}

void FBFont::drawBitmap(int posx, int posy, Bitmap* bitmap) {
	int srcIndex = 0;
	for (int y = 0; y < bitmap->width ; y++) {
		for (int x = 0; x < bitmap->height; x++) {
			srcIndex = (x*4) + (y*4 * bitmap->width);
			setPixel(x+posx, y+posy,
					bitmap->pixels[srcIndex+2] ,
					bitmap->pixels[srcIndex+1],
					bitmap->pixels[srcIndex+0],
                    bitmap->pixels[srcIndex+3]
                    );
		}
	}
}





