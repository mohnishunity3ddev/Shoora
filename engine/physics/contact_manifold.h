#if !defined(CONTACT_MANIFOLD_H)
#define CONTACT_MANIFOLD_H

#include "body.h"
#include "constraint.h"
#include "contact.h"
#include <containers/dynamic_array.h>
#include <defines.h>

// NOTE: Data structure to hold multiple contact points. This will help in getting a stack of boxes stable.
struct manifold
{
    manifold() : A(nullptr), B(nullptr), NumContacts(0) {}

    void AddContact(const contact &Contact);
    void RemoveExpiredContacts();

    void PreSolve(const f32 dt);
    void Solve();
    void PostSolve();

  private:
    static const i32 MAX_CONTACTS = 4;
    contact Contacts[MAX_CONTACTS];
    
    i32 NumContacts = 0;

    shoora_body *A;
    shoora_body *B;

    penetration_constraint_3d PenConstraints[MAX_CONTACTS];

    friend struct manifold_collector;
};

struct manifold_collector
{
    manifold_collector() {}

    void AddContact(const contact &Contact);

    void PreSolve(const f32 dt);
    void Solve();
    void PostSolve();

    void RemoveExpired();
    void Clear();

  public:
    shoora_dynamic_array<manifold> Manifolds;
};

#endif // CONTACT_MANIFOLD_H