/*
.-. .-.  .--.  .----.   .--.  .-. .-..-..----. .-.
| {_} | / {} \ | {}  } / {} \ | |/ / | || {}  }| |
| { } |/  /\  \| .-. \/  /\  \| |\ \ | || .-. \| |
`-' `-'`-'  `-'`-' `-'`-'  `-'`-' `-'`-'`-' `-'`-'
*/

#include "ronin.h"
#include "ronin_matrix.h"
#include "ronin_audio.h"

using namespace RoninEngine::Exception;

namespace RoninEngine
{
    namespace Runtime
    {
        template<typename T>
        void RefMarkNull(Ref<T> object);

        template<>
        void RefMarkNull(GameObjectRef object)
        {
            if(object)
            {
                object.ptr_->m_name = "null";
                object.ptr_->_handle = RefClassType::Null;
            }
        }

        template<>
        void RefMarkNull(ComponentRef object)
        {
            if(object)
            {
                object.ptr_->m_name = "null";
                object.ptr_->_owner = nullptr;
                object.ptr_->_handle = RefClassType::Null;
            }
        }

        template<>
        void RefMarkNull(SpriteRef object)
        {
            if(object)
            {
                object.ptr_->name = "null";
                object.ptr_->surface = nullptr;
                object.ptr_->_handle = RefClassType::Null;
            }
        }

        template<>
        void RefMarkNull(AssetRef asset)
        {
            if(asset->isNull())
                return;

            switch(asset->impl->index)
            {
                case AssetIndex::AS_SPRITE:
                    if(asset->impl->udata._sprites)
                        RoninMemory::free(asset->impl->udata._sprites);
                    break;
                case AssetIndex::AS_AUDIO:
                    if(asset->impl->udata._audioclips)
                        RoninMemory::free(asset->impl->udata._audioclips);
                    break;
                case AssetIndex::AS_MUSIC:
                    if(asset->impl->udata._mus)
                        RoninMemory::free(asset->impl->udata._mus);
                    break;
                case AssetIndex::AS_ATLAS:
                    if(asset->impl->_atlas)
                    {
                        // ReleaseRef(asset.ref->_atlas);
                        asset->impl->_atlas = nullptr;
                    }
                    break;
            }
            RoninMemory::free(asset->impl); // free asset
            asset->impl = nullptr;
            asset->_handle = RefClassType::Null;
        }

        void sepuku_Component(ComponentRef& CND)
        {
            union
            {
                Renderer *renderer;
                Transform *transform;
                Behaviour *script;
                Camera *camera;
                AudioSource *audioSource;
            } _knife;

            Component* candidate = CND.ptr_;

            if(candidate->_owner == nullptr)
            {
                int foo = 0;
            }

#define self (_knife.transform)
            if((self = dynamic_cast<Transform *>(candidate)))
            {
                // Is Unloding world
                if(!currentWorld->irs->requestUnloading)
                {
                    if(self->m_parent)
                    {
                        hierarchy_child_remove(self->m_parent.ptr_, self);
                    }
                    // Parent is off for self Transform
                    hierarchy_childs_remove(self);

                    // picking from matrix
                    if(self->_owner)
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
                    scripts_unbind(self->GetRef<Behaviour>());
                }
            }
#undef self
#define self (_knife.camera)
            else if((self = dynamic_cast<Camera *>(candidate)))
            {
                // Free Camera Resources
                currentWorld->irs->event_camera_changed(StaticCast<Camera>(CND), CameraEvent::CAM_DELETED);
            }
#undef self
#define self (_knife.renderer)
            else if((self = dynamic_cast<Renderer *>(candidate)))
            {
            }
#undef self
#define self (_knife.audioSource)
            else if((self = dynamic_cast<AudioSource*>(candidate)))
            {
                self->setClip(nullptr);
                // Close opened channel
                RoninAudio::closeChannel(self->data->targetChannel);
            }
#undef self

            // Free object
            RefMarkNull(CND);
            RefReleaseSoft(static_cast<RoninPointer*>(CND.ptr_));
            --currentWorld->irs->objects;
        }

        void sepuku_GameObject(GameObjectRef target, std::set<GameObject*> *input)
        {
            GameObjectRef* candidate = &target;
            std::vector<GameObjectRef*> collects {candidate};
            int cursor = 0;
            while(candidate && !candidate->isNull())
            {
                for(TransformRef &t : candidate->ptr_->transform()->hierarchy)
                    collects.push_back(&t->_owner);
                if(!currentWorld->irs->requestUnloading)
                {
                    if(input)
                    {
                        input->erase(candidate->ptr_);
                    }
                    // Send event OnDestroy to GameObject pre Harakiri
                    for(auto event = std::begin(candidate->ptr_->ev_destroy); event != std::end(candidate->ptr_->ev_destroy); ++event)
                    {
                        (*event)(*candidate);
                    }
                    for(ComponentRef &component : candidate->ptr_->m_components)
                    {
                        // Send event OnDestroy to Component object pre Harakiri
                        for(auto event = std::begin(component->ev_destroy); event != std::end(component->ev_destroy); ++event)
                        {
                            (*event)(component);
                        }
                    }
                }
                if(++cursor < collects.size())
                    candidate = collects[cursor];
                else
                    candidate = nullptr;
            }

            //////////////////////////
            /// HARAKIRI COMPONENT ///
            //////////////////////////
            for(GameObjectRef*& next : collects)
            {
                for(ComponentRef& component : next->ptr_->m_components)
                {
                    // HARAKIRI COMPONENT OBJECT
                    sepuku_Component(component);
                }
                if(!currentWorld->irs->requestUnloading)
                {
                    next->ptr_->CancelDestroy();
                }
                // HARAKIRI GAME OBJECT
                RefMarkNull(*next);
                RefReleaseSoft(static_cast<RoninPointer*>(next->ptr_));
                --currentWorld->irs->objects;
                ++currentWorld->irs->_destroyedGameObject;
            }
        }

        int sepuku_run()
        {
#define COLLECTOR (currentWorld->irs->runtimeCollectors)
            currentWorld->irs->_destroyedGameObject = 0;
            if(COLLECTOR)
            {
                std::set<GameObject*> harakiri_candidates;
                std::map<float, std::set<GameObject*>>::iterator range_begin, range_end;
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
                        sepuku_GameObject((*harakiri_candidates.begin())->GetRef<GameObject>(), &harakiri_candidates);
                    } while(!harakiri_candidates.empty());
                }
                if(COLLECTOR->empty())
                {
                    RoninMemory::free(COLLECTOR);
                    COLLECTOR = nullptr;
                }
            }
            return currentWorld->irs->_destroyedGameObject;
#undef COLLECTOR
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
                RoninMemory::alloc_self(provider);
            else
                currentWorld->CancelObjectDestruction(obj);
            t += internal_game_time;
            // So, destroy childrens of the object
            (*provider)[t].insert(obj.ptr_);
#undef provider
        }

    } // namespace Runtime
} // namespace RoninEngine
