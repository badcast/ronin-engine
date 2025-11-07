#include "ronin.h"
#include "ronin_matrix.h"

namespace RoninEngine::Runtime
{

    Renderer::Renderer() : Renderer(DESCRIBE_AS_MAIN_OFF(Renderer))
    {
        Matrix::matrix_update_r(StaticCast<Renderer>(GetRef()));
    }

    Renderer::Renderer(const std::string &name) : Component(DESCRIBE_AS_ONLY_NAME(Renderer))
    {
        DESCRIBE_AS_MAIN(Renderer);
    }
} // namespace RoninEngine::Runtime
