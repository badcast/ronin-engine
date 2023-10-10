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
                        hierarchy_child_remove(self->m_parent, self);

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
                    if(switched_world->irs->_firstRunScripts)
                        switched_world->irs->_firstRunScripts->remove(self);
                    if(switched_world->irs->runtimeScripts)
                        switched_world->irs->runtimeScripts->remove(self);
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

            // Is Unloding world
            if(!switched_world->irs->request_unloading)
            {
                candidate->_type_ = nullptr;
            }

            // Free object
            RoninMemory::free(candidate);
            --switched_world->irs->objects;
        }

        void harakiri_GameObject(GameObject *target)
        {

#ifndef NDEBUG
            if(strcmp(target->_type_, "GameObject"))
            {
                throw ronin_type_error();
            }
#endif

            // destroy childs
            std::list<GameObject *> __stacks;
            while(target)
            {
                for(Transform *t : target->transform()->hierarchy)
                    __stacks.emplace_back(t->_owner);

                for(Component *component : target->m_components)
                    harakiri_Component(component);

                if(!switched_world->irs->request_unloading)
                    // remove extractor flag
                    target->_type_ = nullptr;

                // START FREE OBJECT
                RoninMemory::free(target);
                --switched_world->irs->objects;

                if(!__stacks.empty())
                {
                    target = __stacks.front();
                    __stacks.pop_front();
                }
                else
                    target = nullptr;
            }
        }

        void RuntimeCollector()
        {
            switched_world->irs->_destroyed = 0;
            // Destroy queue objects

            if(switched_world->irs->runtimeCollectors)
            {
                std::map<float, std::set<GameObject *>>::iterator xiter = std::end(*(switched_world->irs->runtimeCollectors)),
                                                                  yiter = std::end(*(switched_world->irs->runtimeCollectors));
                for(std::pair<const float, std::set<GameObject *>> &pair : *(switched_world->irs->runtimeCollectors))
                {
                    // The time for the destruction of the object has not yet come
                    if(pair.first > internal_game_time)
                        break;

                    if(xiter == std::end(*(switched_world->irs->runtimeCollectors)))
                        xiter = yiter = std::begin(*(switched_world->irs->runtimeCollectors));

                    // destroy
                    for(GameObject *target : pair.second)
                    {
                        // run destructor
                        harakiri_GameObject(target);
                        ++(switched_world->irs->_destroyed);
                    }
                    ++yiter;
                }

                if(xiter != yiter)
                    switched_world->irs->runtimeCollectors->erase(xiter, yiter);

                if(switched_world->irs->runtimeCollectors->empty())
                {
                    RoninMemory::free(switched_world->irs->runtimeCollectors);
                    switched_world->irs->runtimeCollectors = nullptr;
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

#define provider (switched_world->irs->runtimeCollectors)
            if(!provider)
            {
                RoninMemory::alloc_self(provider);
            }
            else
            {
                // get error an exists destructed @object
                for(std::pair<const float, std::set<GameObject *>> &mapIter : *provider)
                {
                    auto i = mapIter.second.find(obj);
                    if(i != std::end(mapIter.second))
                    {
                        mapIter.second.erase(i);
                        break;
                    }
                }
            }
            t += internal_game_time;

            // So, destroy childrens of the object
            provider->operator[](t).emplace(obj);

#undef provider
        }

    } // namespace Runtime
} // namespace RoninEngine
