//
// Created by cefika on 9/13/22.
//

#ifndef PROJECT_BASE_FUNCTIONS_H
#define PROJECT_BASE_FUNCTIONS_H


class Function {
public:
    Function() {};

    void loadTv(Model &tvModel, glm::mat4 &model, Shader &shader) {
        model = glm::translate(model, glm::vec3(2.0f, 0.8435f, 3.5f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.0035f, 0.004f, 0.004f));
        shader.setMat4("model", model);
        tvModel.Draw(shader);
    }

    void loadSofa(Model &sofaModel, glm::mat4 &model, Shader &shader) {
        model = glm::translate(model, glm::vec3(2.0f, -0.01f, -3.3f));
        model = glm::scale(model, glm::vec3(1.0f));
        shader.setMat4("model", model);
        sofaModel.Draw(shader);
    }

    void loadDesk(Model &deskModel, glm::mat4 &model, Shader &shader) {
        model = glm::translate(model, glm::vec3(2.0f, 0.005f, 3.5f));
        model = glm::scale(model, glm::vec3(0.02f, 0.04f, 0.02f));
        shader.setMat4("model", model);
        deskModel.Draw(shader);
    }

    void settingUpLight(Shader &lightShader, glm::mat4 &model) {
        glm::vec3 light_positions[] = {
                glm::vec3(0.0f, 1.9f, 3.6f),
                glm::vec3(0.4f, 1.9f, 4.0f),
                glm::vec3(0.0f, 1.9f, 4.4f),
                glm::vec3(-0.4f, 1.9f, 4.0f),

                glm::vec3(0.0f, 3.9f, 3.6f),
                glm::vec3(0.4f, 3.9f, 4.0f),
                glm::vec3(0.0f, 3.9f, 4.4f),
                glm::vec3(-0.4f, 3.9f, 4.0f),

                glm::vec3(0.0f, 5.9f, 3.6f),
                glm::vec3(0.4f, 5.9f, 4.0f),
                glm::vec3(0.0f, 5.9f, 4.4f),
                glm::vec3(-0.4f, 5.9f, 4.0f)
        };
        int n = 12;

        for (int i = 0; i < n; ++i) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, light_positions[i]);
            model = glm::scale(model, glm::vec3(0.1f));
            lightShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        for (int i = 0; i < n; ++i) {
            model = glm::mat4(1.0f);
            light_positions[i].x += 4.0f;
            model = glm::translate(model, light_positions[i]);
            model = glm::scale(model, glm::vec3(0.1f));
            lightShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

//        model = glm::mat4(1.0f);
//        model = glm::translate(model, glm::vec3(-5.5f, 4.55f, -2.65f));
//        model = glm::scale(model, glm::vec3(0.1f));
//        lightShader.setMat4("model", model);
//
//        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    
    void settingUpRoof(Shader &shader, glm::mat4 &model) {
        int n = 3;
        float x = 5.0f;

        for (int i = 0; i < n; ++i) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, 6.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.4f, 0.4f, 10.0f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            x -= 5.0f;
        }
    }

    void settingUpTilesInWall(Shader &shader, glm::mat4 &model) {
        float tiles_in_wall_x_positions[] = {
                6.5f, 4.75f, 3.0f, 1.25f, -0.5f, -2.25f, -4.0f, -5.75f, -7.5f
        };
        float y = 6.0f, z = -5.5f;
        int n = 9;

        for (int i = 0; i < n; ++i) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(tiles_in_wall_x_positions[i], y, z));
            model = glm::scale(model, glm::vec3(1.5f, 0.1f, 1.5f));
            shader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        z *= -1;
        for (int i = 0; i < n; ++i) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(tiles_in_wall_x_positions[i], y, z));
            model = glm::scale(model, glm::vec3(1.5f, 0.1f, 1.5f));
            shader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    void settingUpTilesInPillar(Shader &shader, glm::mat4 &model) {
        float x = 0.0f, y, z = 4.0f;
        int m = 2, n = 4;

        for (int j = 0; j < m; ++j) {

            y = 0.0f;
            for (int i = 0; i < n; ++i) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, y, z));
                // 1.1f -> x, z
                model = glm::scale(model, glm::vec3(1.0f, 0.1f, 1.0f));
                shader.setMat4("model", model);

                glDrawArrays(GL_TRIANGLES, 0, 36);
                y += 2.0f;
            }
            x = 4.0f;
            z = 4.0f;
        }
    }

    void settingUpFloor(Shader &shader, glm::mat4 &model, unsigned int floor) {
        glBindTexture(GL_TEXTURE_2D, floor);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        /*model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 6.0f, 0.0f));
        //model = glm::scale(model,glm::vec3(0.0f, 1.0f, 1.0f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);*/

//        model = glm::mat4(1.0f);
//        model = glm::translate(model, glm::vec3(0.0f, 12.0f, 0.0f));
//        shader.setMat4("model", model);
//        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    void settingUpPillar(Shader& shader, glm::mat4 &model, unsigned int stone) {
        int n = 6;
        float y = 0.5f;

        glBindTexture(GL_TEXTURE_2D, stone);
        for (int i = 0; i < n; ++i) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, y, 4.0f));
            model = glm::scale(model, glm::vec3(0.7f, 1.0f, 0.7f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            y += 1.0f;
        }

        y = 0.5f;
        for (int i = 0; i < n; ++i) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(4.0f, y, 4.0f));
            model = glm::scale(model, glm::vec3(0.7f, 1.0f, 0.7f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            y += 1.0f;
        }
    }

    void settingUpWall(Shader &shader, glm::mat4 &model, unsigned int tile, unsigned int wall, float height) {
        float x, y = height, z;
        int m = 6, n = 14, k = 11;

        for (int j = 0; j < m; ++j) {
            glBindTexture(GL_TEXTURE_2D, wall);
            if (j == 1 || j == 4) {
                glBindTexture(GL_TEXTURE_2D, tile);
            }

            x = 6.5f, z = -5.5f;
            for (int i = 0; i < n; ++i) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, y, z));
                shader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                x -= 1.0f;
            }

            for (int i = 0; i < k; ++i) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, y, z));
                shader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                z += 1.0f;
            }

            for (int i = 0; i <= n; ++i) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, y, z));
                shader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                x += 1.0f;
            }

            y += 1.0f;
        }
    }
};

#endif //PROJECT_BASE_FUNCTIONS_H
