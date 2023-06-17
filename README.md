# ronin-engine framework

```mermaid
graph TB

_platforms(GNU/Linux, Windows, MacOS, etc.)
_ronin_engine(Ronin Engine)
_libSDL2(libSDL2 components)
_justparser(just-parser)

_platforms --> _libSDL2 --> _ronin_engine
_platforms --> _ronin_engine
_justparser --> _conf(Ronin Engine Configs)
_ronin_engine --> _conf
_ronin_engine --> _rgi(Init Game) --> _rgs(Game Input/Ouput) --> _rgw(World Load) --> _rgsi(Simulation) --> _rgc(Closing and unitializing) --> Ending[End]
```

## [Ru]ssian Language
> Цель проекта: Обеспечение минимальной потребности для рендеринга объектов на большинства платформах (GNU/Linux, Windows, MacOS).
------------
## Зависимости
- **SDL2** - соопровождение движка визуализациями, так-же обеспечением движка ресурсами.
- **just-parser** - библиотека по управлению конфигурациями ([just-parser](https://github.com/badcast/just-parser "just-parser"))

------------
#### Как собрать движок?
```bash
#cloning sources
git clone "https://github.com/badcast/ronin-engine.git"
#into directory
cd ronin-engine
#make build directory
mkdir build
#into directory
cd build
#Building from type Release (see Debug, MinSizeRel, Release, RelWithDebInfo) from up directory (..)
cmake -DCMAKE_BUILD_TYPE=Release ..
```

