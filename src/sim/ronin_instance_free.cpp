#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Exception;

namespace RoninEngine
{
    namespace Runtime
    {
        void harakiri_Component(Component *candidate)
        {
            union
            {
                Renderer *renderer;
                Transform *transform;
                Behaviour *script;
                Camera *camera;
            } _knife;
#define self (_knife.transform)
            if((self = dynamic_cast<Transform *>(candidate)))
            {
                // Is Unloding world
                if(!switched_world->irs->request_unloading)
                {
                    if(self->m_parent)
                    {
                        hierarchy_child_remove(self->m_parent, self);
                    }
                    // Parent is off for self Transform
                    hierarchy_childs_remove(self);

                    // picking from matrix
                    Matrix::matrix_remove(self);
                }
            }
#undef self
#define self (_knife.script)
            else if((self = dynamic_cast<Behaviour *>(candidate)))
            {
                // Run Script Destroy
                self->OnDestroy();

                // Is Unloding world
                if(!switched_world->irs->request_unloading)
                {
                    // Run last script object
                    scripts_unbind(self);
                }
            }
#undef self
#define self (_knife.camera)
            else if((self = dynamic_cast<Camera *>(candidate)))
            {
                // Free Camera Resources
                switched_world->irs->event_camera_changed(self, CameraEvent::CAM_DELETED);
            }
#undef self
#define self (_knife.renderer)
            else if((self = dynamic_cast<Renderer *>(candidate)))
            {
            }
#undef self

            // Is Unloding World
            if(!switched_world->irs->request_unloading)
            {
                candidate->_type_ = nullptr;
            }

            // Free object
            RoninMemory::free(candidate);
            --switched_world->irs->objects;
        }

        void harakiri_GameObject(GameObject *target, std::set<GameObject *> *input)
        {
#ifndef NDEBUG
            if(strcmp(target->_type_, "GameObject"))
            {
                throw ronin_type_error();
            }
#endif
            // COLLECT +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            std::vector<GameObject *> collects {target};
            int cursor = 0;

            while(target)
            {
                for(Transform *t : target->transform()->hierarchy)
                    collects.emplace_back(t->_owner);

                if(!switched_world->irs->request_unloading)
                {
                    if(input)
                    {
                        input->erase(target);
                    }

                    // Send event OnDestroy to GameObject pre Harakiri
                    for(auto event = std::begin(target->ev_destroy); event != std::end(target->ev_destroy); ++event)
                    {
                        (*event)(target);
                    }

                    for(Component *component : target->m_components)
                    {
                        // Send event OnDestroy to Component object pre Harakiri
                        for(auto event = std::begin(component->ev_destroy); event != std::end(component->ev_destroy); ++event)
                        {
                            (*event)(component);
                        }
                    }
                }

                if(++cursor < collects.size())
                {
                    target = collects[cursor];
                }
                else
                    target = nullptr;
            }

            // HARAKIRI --------------------------------------------------------------------------------------------------------------------
            for(GameObject *next : collects)
            {
                for(Component *component : next->m_components)
                {
                    // HARAKIRI COMPONENT OBJECT
                    harakiri_Component(component);
                }

                if(!switched_world->irs->request_unloading)
                {
                    next->CancelDestroy();
                    next->_type_ = nullptr;
                }

                // HARAKIRI GAME OBJECT
                RoninMemory::free(next);
                --switched_world->irs->objects;
                ++switched_world->irs->_destroyedGameObject;
            }
        }

        void Bushido_Tradition_Harakiri()
        {
            switched_world->irs->_destroyedGameObject = 0;

#define COLLECTOR (switched_world->irs->runtimeCollectors)

            if(COLLECTOR)
            {
                std::map<float, std::set<GameObject *>>::iterator range_begin, range_end;
                std::set<GameObject *> harakiri_candidates;

                // std::pair<const float, std::set<GameObject *>> &pair
                range_begin = range_end = std::begin(*COLLECTOR);
                for(; range_end != std::end(*COLLECTOR);)
                {
                    // The time for the destruction of the object has not yet come
                    if((*range_end).first > internal_game_time)
                        break;

                    harakiri_candidates.merge(range_end->second);
                    ++range_end;
                }

                if(range_begin != range_end)
                {
                    COLLECTOR->erase(range_begin, range_end);
                    do
                    {
                        harakiri_GameObject(*harakiri_candidates.begin(), &harakiri_candidates);
                    } while(!harakiri_candidates.empty());
                }

                if(COLLECTOR->empty())
                {
                    RoninMemory::free(COLLECTOR);
                    COLLECTOR = nullptr;
                }
            }
        }

        void Destroy(GameObject *obj)
        {
            Destroy(obj, 0);
        }

        void Destroy(GameObject *obj, float t)
        {
            if(!obj || !switched_world || t < 0)
                throw std::bad_exception();

                // #error FIXME ____ ADD DESTROY OBJECTS TO QUEUE

#define provider (switched_world->irs->runtimeCollectors)
            if(!provider)
            {
                RoninMemory::alloc_self(provider);
            }
            else
            {
                switched_world->CancelObjectDestruction(obj);
            }
            t += internal_game_time;

            // So, destroy childrens of the object
            provider->operator[](t).insert(obj);

#undef provider
        }

    } // namespace Runtime
} // namespace RoninEngine
