#include "contact_manifold.h"

void
manifold::AddContact(const contact &NewContact)
{
    contact Contact = NewContact;
    // NOTE: Making sure that the order of the bodies is the correct one according to the order in this manifold
    // contact. Whenever this call is made, we have a guarantee that the bodies involved in the newContact is the
    // same pair as that in the contact stored in this manifold(if it is indeed present), what is not guaranteed is
    // the order of A and B.
    if(Contact.ReferenceBodyA != this->A || Contact.IncidentBodyB != this->B)
    {
        Contact.ReferenceHitPointA_LocalSpace = NewContact.IncidentHitPointB_LocalSpace;
        Contact.IncidentHitPointB_LocalSpace = NewContact.ReferenceHitPointA_LocalSpace;

        Contact.ReferenceBodyA = this->A;
        Contact.IncidentBodyB = this->B;
    }

    // NOTE: If this new contact is very close close to the old contact, we would like to keep the old contact
    // since it is already a stable value and it helps in warm starting.
    for(i32 i = 0; i < this->NumContacts; ++i)
    {
        shoora_body *bodyA = this->Contacts[i].ReferenceBodyA;
        shoora_body *bodyB = this->Contacts[i].IncidentBodyB;

        shu::vec3f OldContactPosWS_A = bodyA->LocalToWorldSpace(this->Contacts[i].ReferenceHitPointA_LocalSpace);
        shu::vec3f OldContactPosWS_B = bodyB->LocalToWorldSpace(this->Contacts[i].IncidentHitPointB_LocalSpace);

        shu::vec3f NewContactPosWS_A = Contact.ReferenceBodyA->LocalToWorldSpace(Contact.ReferenceHitPointA_LocalSpace);
        shu::vec3f NewContactPosWS_B = Contact.IncidentBodyB->LocalToWorldSpace(Contact.IncidentHitPointB_LocalSpace);

        shu::vec3f aa = NewContactPosWS_A - OldContactPosWS_A;
        shu::vec3f bb = NewContactPosWS_B - OldContactPosWS_B;

        f32 DistThreshold = .02f;
        if(aa.SqMagnitude() < DistThreshold*DistThreshold) {
            return;
        }
        if(bb.SqMagnitude() < DistThreshold*DistThreshold) {
            return;
        }
    }

    // NOTE: If the contact are MAX_CONTACT(the allowed number of contact are at MAX), then keep the contacts that
    // are furthest away from each other. So we remove the one contact which is the closest to the average of all
    // the 5 max contact points.
    // NOTE: ANother strategy could be to keep the contact with the highest amount of penetration and then keep
    // contacts that are furthest away from it.
    i32 NewContactSlot = this->NumContacts;
    if(NewContactSlot >= MAX_CONTACTS)
    {
        shu::vec3f Average = this->Contacts[0].ReferenceHitPointA_LocalSpace;
        Average += this->Contacts[1].ReferenceHitPointA_LocalSpace;
        Average += this->Contacts[2].ReferenceHitPointA_LocalSpace;
        Average += this->Contacts[3].ReferenceHitPointA_LocalSpace;
        Average += Contact.ReferenceHitPointA_LocalSpace;
        Average *= 0.2f;

        f32 MinDistance = (Average - Contact.ReferenceHitPointA_LocalSpace).SqMagnitude();
        i32 NewIdx = -1;
        for(i32 i = 0; i < MAX_CONTACTS; ++i)
        {
            f32 DistanceSquared = (this->Contacts[i].ReferenceHitPointA_LocalSpace - Average).SqMagnitude();
            if(DistanceSquared < MinDistance)
            {
                MinDistance = DistanceSquared;
                NewIdx = i;
            }
        }

        if(NewIdx != -1) {
            NewContactSlot = NewIdx;
        } else {
            return;
        }
    }

    this->Contacts[NewContactSlot] = Contact;

    // NOTE: Update the Penetration Constraint for the new contact and clear its cached lagrange multipliers.
    auto *PenetrationConstraint = this->PenConstraints + NewContactSlot;
    PenetrationConstraint->A = Contact.ReferenceBodyA;
    PenetrationConstraint->B = Contact.IncidentBodyB;
    PenetrationConstraint->AnchorPointLS_A = Contact.ReferenceHitPointA_LocalSpace;
    PenetrationConstraint->AnchorPointLS_B = Contact.IncidentHitPointB_LocalSpace;

    // NOTE: Normal in Body A's Space.
    shu::vec3f NormalLS_A = shu::QuatRotateVec(shu::QuatConjugate(this->A->Rotation), -Contact.Normal);
    NormalLS_A = shu::Normalize(NormalLS_A);
    PenetrationConstraint->Normal_LocalSpaceA = NormalLS_A;
    PenetrationConstraint->PreviousFrameLambdas.Zero();

    if(NewContactSlot == this->NumContacts)
    {
        ++this->NumContacts;
    }
}

// NOTE: Since we are caching contacts on a frame by frame basis, there are some contacts which are from old
// frames. as the simulation runs, there will be some contacts that have to be invalidated if they have been
// drifted off to far.
void
manifold::RemoveExpiredContacts()
{
    // NOTE: Remove any contacts that have drifted too far.
    for(i32 i = 0; i < this->NumContacts; ++i)
    {
        contact &Contact = this->Contacts[i];

        shoora_body *A = Contact.ReferenceBodyA;
        shoora_body *B = Contact.IncidentBodyB;

        // NOTE: A->LocalSpaceToWS returns the positon of the old contact points currently in the WS according to
        // the changed a's and b's position.
        shu::vec3f CurrentContactPointA = A->LocalToWorldSpace(Contact.ReferenceHitPointA_LocalSpace);
        shu::vec3f CurrentContactPointB = B->LocalToWorldSpace(Contact.IncidentHitPointB_LocalSpace);

        shu::vec3f ContactNormalInLocalSpaceA = this->PenConstraints[i].Normal_LocalSpaceA;
        shu::vec3f CurrentContactNormalWS = shu::QuatRotateVec(A->Rotation, ContactNormalInLocalSpaceA);
        CurrentContactNormalWS = shu::Normalize(CurrentContactNormalWS);

        // NOTE: Calculate the current penetration depth and normal to see if in current frame, the contact has
        // become invalid since it has drifted too far apart than the information already available in our cached
        // contact points.
        shu::vec3f AB = CurrentContactPointB - CurrentContactPointA;
        f32 CurrentPenetrationDepth = CurrentContactNormalWS.Dot(AB);
        shu::vec3f abNormal = CurrentContactNormalWS * CurrentPenetrationDepth;
        shu::vec3f abTangent = AB - abNormal;

        f32 DistThreshold = 0.02f;
        // NOTE: If the Contact Normal currently aligns to a large extent with the contact normal associated with
        // the old contact, then it's okay to keep it since its not too far off.
        if(abTangent.SqMagnitude() < DistThreshold*DistThreshold && CurrentPenetrationDepth <= 0.0f)
        {
            continue;
        }

        // NOTE: Here, the contact has drifted off and needs to be invalidated.
        for(i32 j = 0; j < MAX_CONTACTS - 1; ++j)
        {
            this->PenConstraints[j] = this->PenConstraints[j + 1];
            this->Contacts[j] = this->Contacts[j + 1];
            if(j >= this->NumContacts)
            {
                this->PenConstraints[j].PreviousFrameLambdas.Zero();
            }
        }
        this->NumContacts--;
        i--;
    }
}

void
manifold::PreSolve(const f32 dt)
{
    for(i32 i = 0; i < this->NumContacts; ++i)
    {
        this->PenConstraints[i].PreSolve(dt);
    }
}

void
manifold::Solve()
{
    for(i32 i = 0; i < this->NumContacts; ++i)
    {
        this->PenConstraints[i].Solve();
    }
}

void
manifold::PostSolve()
{
    for(i32 i = 0; i < this->NumContacts; ++i)
    {
        this->PenConstraints[i].PostSolve();
    }
}

void
manifold_collector::AddContact(const contact &Contact)
{
    i32 FoundIdx = -1;
    // NOTE: We have found a contact in the cachedContacts that deals with the same pair of bodies which is there
    for(i32 i = 0; i < this->Manifolds.size(); ++i)
    {
        const manifold &Manifold = this->Manifolds[i];
        b32 HasA = (Manifold.A == Contact.ReferenceBodyA || Manifold.B == Contact.ReferenceBodyA);
        b32 HasB = (Manifold.A == Contact.IncidentBodyB || Manifold.B == Contact.IncidentBodyB);
        if(HasA && HasB)
        {
            FoundIdx = i;
            break;
        }
    }

    if(FoundIdx >= 0)
    {
        this->Manifolds[FoundIdx].AddContact(Contact);
    }
    else
    {
        manifold Manifold;
        Manifold.A = Contact.ReferenceBodyA;
        Manifold.B = Contact.IncidentBodyB;

        Manifold.AddContact(Contact);
        this->Manifolds.emplace_back(Manifold);
    }
}

void
manifold_collector::RemoveExpired()
{
    for(i32 i = (i32)this->Manifolds.size() - 1; i >= 0; --i)
    {
        manifold &Manifold = this->Manifolds[i];
        Manifold.RemoveExpiredContacts();

        if(Manifold.NumContacts == 0)
        {
            this->Manifolds.erase(i);
        }
    }
}

void
manifold_collector::Clear()
{
    this->Manifolds.Clear();
}

void
manifold_collector::PreSolve(const f32 dt)
{
    for(i32 i = 0; i < this->Manifolds.size(); ++i)
    {
        this->Manifolds[i].PreSolve(dt);
    }
}

void
manifold_collector::Solve()
{
    for(i32 i = 0; i < this->Manifolds.size(); ++i)
    {
        this->Manifolds[i].Solve();
    }
}

void
manifold_collector::PostSolve()
{
    for(i32 i = 0; i < this->Manifolds.size(); ++i)
    {
        this->Manifolds[i].PostSolve();
    }
}