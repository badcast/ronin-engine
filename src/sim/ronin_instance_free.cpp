#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Exception;

namespace RoninEngine
{
    namespace Runtime
    {
        void sepuku_Component(ComponentRef& CND)
        {
            union
            {
                Renderer *renderer;
                Transform *transform;
                Behaviour *script;
                Camera *camera;
            } _knife;

            Component* candidate = CND.get();

#define self (_knife.transform)
            if((self = dynamic_cast<Transform *>(candidate)))
            {
                // Is Unloding world
                if(!currentWorld->irs->requestUnloading)
                {
                    if(self->m_parent)
                    {
                        hierarchy_child_remove(self->m_parent.get(), self);
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
                if(!currentWorld->irs->requestUnloading)
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
                currentWorld->irs->event_camera_changed(self, CameraEvent::CAM_DELETED);
            }
#undef self
#define self (_knife.renderer)
            else if((self = dynamic_cast<Renderer *>(candidate)))
            {
            }
#undef self

#if RONIN_USE_TYPESTR
            // Is Unloding World
            if(!switched_world->irs->request_unloading)
            {
                candidate->_type_ = nullptr;
            }
#endif

            // Free object
            //RoninMemory::free(candidate);
            // ReleaseRef(CND);
            candidate->m_name = "null";
            candidate->_type_ = nullptr;

            --currentWorld->irs->objects;
        }

        void sepuku_GameObject(GameObject *target, std::set<GameObject *> *input)
        {
#ifndef RONIN_USE_TYPESTR &&NDEBUG
            if(strcmp(target->_type_, "GameObject"))
            {
                throw ronin_type_error();
            }
#endif
            // COLLECT
            std::vector<GameObject *> collects {target};
            int cursor = 0;

            while(target)
            {
                for(TransformRef &t : target->transform()->hierarchy)
                    collects.emplace_back(t->_owner.ptr_);

                if(!currentWorld->irs->requestUnloading)
                {
                    if(input)
                    {
                        input->erase(target);
                    }

                    // Send event OnDestroy to GameObject pre Harakiri
                    for(auto event = std::begin(target->ev_destroy); event != std::end(target->ev_destroy); ++event)
                    {
                        (*event)(target->GetRef<GameObject>());
                    }

                    for(ComponentRef &component : target->m_components)
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

            //////////////////////////
            /// HARAKIRI COMPONENT ///
            //////////////////////////
            for(GameObject *next : collects)
            {
                for(ComponentRef component : next->m_components)
                {
                    // HARAKIRI COMPONENT OBJECT
                    sepuku_Component(component);
                }

                if(!currentWorld->irs->requestUnloading)
                {
                    next->CancelDestroy();

#if RONIN_USE_TYPESTR
                    next->_type_ = nullptr;
#endif
                }

                // HARAKIRI GAME OBJECT
                //RoninMemory::free(next);
                next->_type_ = nullptr;
                next->m_name = "null";

                --currentWorld->irs->objects;
                ++currentWorld->irs->_destroyedGameObject;
            }
        }

        int sepuku_run()
        {
            currentWorld->irs->_destroyedGameObject = 0;

#define COLLECTOR (currentWorld->irs->runtimeCollectors)

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
                        sepuku_GameObject(*harakiri_candidates.begin(), &harakiri_candidates);
                    } while(!harakiri_candidates.empty());
                }

                if(COLLECTOR->empty())
                {
                    RoninMemory::free(COLLECTOR);
                    COLLECTOR = nullptr;
                }
            }
#undef COLLECTOR

            return currentWorld->irs->_destroyedGameObject;
        }

        void Destroy(GameObjectRef obj)
        {
            Destroy(obj, 0);
        }

        void Destroy(GameObjectRef obj, float t)
        {
            if(obj.isNull() || !currentWorld || t < 0)
                throw std::bad_exception();

#define provider (currentWorld->irs->runtimeCollectors)
            if(!provider)
            {
                RoninMemory::alloc_self(provider);
            }
            else
            {
                currentWorld->CancelObjectDestruction(obj);
            }
            t += internal_game_time;

            // So, destroy childrens of the object
            provider->operator[](t).insert(obj.get());
#undef provider
        }

    } // namespace Runtime
} // namespace RoninEngine
