#include <gtest/gtest.h>
#include "../src/core/type_system.h"

using namespace hotc;

class TypeSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        types_ = std::make_unique<TypeSystem>();
    }

    std::unique_ptr<TypeSystem> types_;
};

TEST_F(TypeSystemTest, GetBuiltinTypes) {
    EXPECT_NE(types_->GetObjectType(), nullptr);
    EXPECT_NE(types_->GetInt32Type(), nullptr);
    EXPECT_NE(types_->GetFloatType(), nullptr);
    EXPECT_NE(types_->GetStringType(), nullptr);
}

TEST_F(TypeSystemTest, RegisterCustomType) {
    TypeInfo* custom = types_->RegisterType("MyClass", TypeKind::Class, 8);
    EXPECT_NE(custom, nullptr);
    EXPECT_EQ(custom->name, "MyClass");
    EXPECT_EQ(custom->kind, TypeKind::Class);
    EXPECT_EQ(custom->size, 8);
}

TEST_F(TypeSystemTest, RegisterClassType) {
    TypeInfo* game_object = types_->RegisterClassType("GameObject", types_->GetObjectType());
    EXPECT_NE(game_object, nullptr);
    EXPECT_EQ(game_object->base_type, types_->GetObjectType());
}

TEST_F(TypeSystemTest, RegisterStructType) {
    TypeInfo* vector3 = types_->RegisterStructType("Vector3", 12);
    EXPECT_NE(vector3, nullptr);
    EXPECT_EQ(vector3->kind, TypeKind::Struct);
    EXPECT_EQ(vector3->size, 12);
}

TEST_F(TypeSystemTest, RegisterField) {
    TypeInfo* game_object = types_->RegisterClassType("GameObject");
    types_->RegisterField(game_object, "name", types_->GetStringType(), 0);

    FieldInfo* field = types_->GetField(game_object, "name");
    EXPECT_NE(field, nullptr);
    EXPECT_EQ(field->name, "name");
    EXPECT_EQ(field->type, types_->GetStringType());
}

TEST_F(TypeSystemTest, RegisterMethod) {
    TypeInfo* game_object = types_->RegisterClassType("GameObject");
    MethodInfo method;
    method.name = "GetName";
    method.return_type = types_->GetStringType();
    method.is_virtual = false;
    method.is_static = false;

    types_->RegisterMethod(game_object, method);

    MethodInfo* found = types_->GetMethod(game_object, "GetName");
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(found->name, "GetName");
}

TEST_F(TypeSystemTest, MethodInheritance) {
    TypeInfo* base = types_->RegisterClassType("Base");
    TypeInfo* derived = types_->RegisterClassType("Derived", base);

    MethodInfo method;
    method.name = "DoSomething";
    method.return_type = types_->GetObjectType();
    types_->RegisterMethod(base, method);

    MethodInfo* found = types_->GetMethod(derived, "DoSomething");
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(found->name, "DoSomething");
}
