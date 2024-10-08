#ifndef NOMAD_ENTITY_HPP
#define NOMAD_ENTITY_HPP

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <bitset>
#include <array>
#include <queue>
#include <functional>
#include <set>
#include <glad/glad.h>
#include <glm/ext.hpp>

constexpr std::size_t MAX_COMPONENTS = 32;
constexpr std::size_t MAX_ENTITIES = 5000;

using ComponentType = std::uint8_t;
using Signature = std::bitset<MAX_COMPONENTS>;

class Entity
{
public:
    Entity(int id) : mId(id) {}
    int id() const { return mId; }

    bool operator<(const Entity &other) const
    {
        return mId < other.mId;
    }

private:
    int mId;
};

// Component Array Interface
class IComponentArray
{
public:
    virtual ~IComponentArray() = default;
    virtual void entityDestroyed(Entity entity) = 0;
};

template <typename T>
class ComponentArray : public IComponentArray
{
public:
    void insertData(Entity entity, T component)
    {
        size_t newIndex = mSize;
        mEntityToIndexMap[entity.id()] = newIndex;
        mIndexToEntityMap[newIndex] = entity.id();
        mComponentArray[newIndex] = component;
        ++mSize;
    }

    void removeData(Entity entity)
    {
        size_t indexOfRemovedEntity = mEntityToIndexMap[entity.id()];
        size_t indexOfLastElement = mSize - 1;
        mComponentArray[indexOfRemovedEntity] = mComponentArray[indexOfLastElement];

        Entity entityOfLastElement(mIndexToEntityMap[indexOfLastElement]);
        mEntityToIndexMap[entityOfLastElement.id()] = indexOfRemovedEntity;
        mIndexToEntityMap[indexOfRemovedEntity] = entityOfLastElement.id();

        mEntityToIndexMap.erase(entity.id());
        mIndexToEntityMap.erase(indexOfLastElement);

        --mSize;
    }

    T &getData(Entity entity)
    {
        return mComponentArray[mEntityToIndexMap[entity.id()]];
    }

    void entityDestroyed(Entity entity) override
    {
        if (mEntityToIndexMap.find(entity.id()) != mEntityToIndexMap.end())
        {
            removeData(entity);
        }
    }

private:
    std::array<T, MAX_ENTITIES> mComponentArray;
    std::unordered_map<int, size_t> mEntityToIndexMap;
    std::unordered_map<size_t, int> mIndexToEntityMap;
    size_t mSize = 0;
};

class ComponentManager
{
public:
    template <typename T>
    void registerComponent()
    {
        const char *typeName = typeid(T).name();
        mComponentTypes.insert({typeName, mNextComponentType});
        mComponentArrays.insert({typeName, std::make_shared<ComponentArray<T>>()});
        ++mNextComponentType;
    }

    template <typename T>
    ComponentType getComponentType()
    {
        const char *typeName = typeid(T).name();
        return mComponentTypes[typeName];
    }

    template <typename T>
    void addComponent(Entity entity, T component)
    {
        getComponentArray<T>()->insertData(entity, component);
    }

    template <typename T>
    void removeComponent(Entity entity)
    {
        getComponentArray<T>()->removeData(entity);
    }

    template <typename T>
    T &getComponent(Entity entity)
    {
        return getComponentArray<T>()->getData(entity);
    }

    void entityDestroyed(Entity entity)
    {
        for (auto const &pair : mComponentArrays)
        {
            auto const &component = pair.second;
            component->entityDestroyed(entity);
        }
    }

private:
    std::unordered_map<const char *, ComponentType> mComponentTypes{};
    std::unordered_map<const char *, std::shared_ptr<IComponentArray>> mComponentArrays{};
    ComponentType mNextComponentType{};

    template <typename T>
    std::shared_ptr<ComponentArray<T>> getComponentArray()
    {
        const char *typeName = typeid(T).name();
        return std::static_pointer_cast<ComponentArray<T>>(mComponentArrays[typeName]);
    }
};

class EntityManager
{
public:
    EntityManager()
    {
        for (int i = 0; i < MAX_ENTITIES; ++i)
        {
            mAvailableEntities.push(i);
        }
    }

    Entity createEntity()
    {
        int id = mAvailableEntities.front();
        mAvailableEntities.pop();
        ++mLivingEntityCount;
        return Entity(id);
    }

    void destroyEntity(Entity entity)
    {
        mSignatures[entity.id()].reset();
        mAvailableEntities.push(entity.id());
        --mLivingEntityCount;
    }

    void setSignature(Entity entity, Signature signature)
    {
        mSignatures[entity.id()] = signature;
    }

    Signature getSignature(Entity entity)
    {
        return mSignatures[entity.id()];
    }

private:
    std::queue<int> mAvailableEntities{};
    std::array<Signature, MAX_ENTITIES> mSignatures{};
    uint32_t mLivingEntityCount{};
};

class System
{
public:
    std::set<Entity> mEntities;
};

class SystemManager
{
public:
    template <typename T>
    std::shared_ptr<T> registerSystem()
    {
        const char *typeName = typeid(T).name();
        auto system = std::make_shared<T>();
        mSystems.insert({typeName, system});
        return system;
    }

    template <typename T>
    void setSignature(Signature signature)
    {
        const char *typeName = typeid(T).name();
        mSignatures.insert({typeName, signature});
    }

    void entityDestroyed(Entity entity)
    {
        for (auto const &pair : mSystems)
        {
            auto const &system = pair.second;
            system->mEntities.erase(entity);
        }
    }

    void entitySignatureChanged(Entity entity, Signature entitySignature)
    {
        for (auto const &pair : mSystems)
        {
            auto const &type = pair.first;
            auto const &system = pair.second;
            auto const &systemSignature = mSignatures[type];

            if ((entitySignature & systemSignature) == systemSignature)
            {
                system->mEntities.insert(entity);
            }
            else
            {
                system->mEntities.erase(entity);
            }
        }
    }

private:
    std::unordered_map<const char *, Signature> mSignatures{};
    std::unordered_map<const char *, std::shared_ptr<System>> mSystems{};
};

class ECS
{
public:
    ECS() = default;
    ~ECS() = default;
    ECS(const ECS &) = delete;
    ECS &operator=(const ECS &) = delete;
    ECS(ECS &&other) noexcept = default;
    ECS &operator=(ECS &&other) noexcept = default;
    void init()
    {
        mComponentManager = std::make_unique<ComponentManager>();
        mEntityManager = std::make_unique<EntityManager>();
        mSystemManager = std::make_unique<SystemManager>();
    }

    EntityManager *getEntityManager() { return mEntityManager.get(); }

    // Entity methods
    Entity createEntity()
    {
        return mEntityManager->createEntity();
    }

    void destroyEntity(Entity entity)
    {
        mEntityManager->destroyEntity(entity);
        mComponentManager->entityDestroyed(entity);
        mSystemManager->entityDestroyed(entity);
    }

    // Component methods
    template <typename T>
    void registerComponent()
    {
        mComponentManager->registerComponent<T>();
    }

    template <typename T>
    void addComponent(Entity entity, T component)
    {
        mComponentManager->addComponent<T>(entity, component);

        auto signature = mEntityManager->getSignature(entity);
        signature.set(mComponentManager->getComponentType<T>(), true);
        mEntityManager->setSignature(entity, signature);

        mSystemManager->entitySignatureChanged(entity, signature);
    }

    template <typename T>
    void removeComponent(Entity entity)
    {
        mComponentManager->removeComponent<T>(entity);

        auto signature = mEntityManager->getSignature(entity);
        signature.set(mComponentManager->getComponentType<T>(), false);
        mEntityManager->setSignature(entity, signature);

        mSystemManager->entitySignatureChanged(entity, signature);
    }

    template <typename T>
    T &getComponent(Entity entity)
    {
        return mComponentManager->getComponent<T>(entity);
    }

    template <typename T>
    ComponentType getComponentType()
    {
        return mComponentManager->getComponentType<T>();
    }

    // System methods
    template <typename T>
    std::shared_ptr<T> registerSystem()
    {
        return mSystemManager->registerSystem<T>();
    }

    template <typename T>
    void setSystemSignature(Signature signature)
    {
        mSystemManager->setSignature<T>(signature);
    }

private:
    std::unique_ptr<ComponentManager> mComponentManager;
    std::unique_ptr<EntityManager> mEntityManager;
    std::unique_ptr<SystemManager> mSystemManager;
};

#endif
