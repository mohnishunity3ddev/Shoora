 **1. Linear Collision Resolution(Does not involve rotations)**
<--The aim is to calculate the Impulse vector acting on the two colliding bodies in this contact-->
Impulse(J) acting on both bodies will be along the collision contact normal vector.
Change in momentum in body A is dP(a) = Ma*V'a - Ma*Va. dP(a) is also Impulse acting along the collision normal.
Hence,  **J.N = Ma*V'a - Ma*Va; V'a = Va + (J.N/Ma) <-- (1)**
For body B, the impulse will be acting in the opposite direction as that of body A.
Hence, **-J.N = Mb*V'b - Mb*Vb; V'b = Vb - (J.N/Mb) <-- (2)**
Relative velocity of a w.r.t. b is Va - Vb pre-collision -> VRela = Va - Vb
Relative velocity post-collision of a w.r.t. b will be V'a - V'b -> V'Rela = V'a - V'b
For elastic collisions => V'Rela = -VRela,
for normal scenarios => **V'Rela = -E*VRela <---(3)**
where E is the coefficient of restitution, which is already present in the body structs.
**Subtracting (2) from (1), we get: V'Rela = VRela + (J.N)(1/Ma + 1/Mb)**
We want to find the relative velocity along the collision normal, so doing a dot product on both sides:
V'Rela.N = VRela.N + J.N(1/Ma + 1/Mb)N
**Substituting (3) here, we get:**
**-E*VRela.N = VRela.N + (J.N)(1/Ma + 1/Mb)N**
(- 1 - E)*VRela.N / (1/Ma + 1/Mb)*(N.N) = J
N.N = 1 since N is unit vector, we get:
**J = (1 - E)*VRela.N / (1/Ma + 1/Mb)**
This is the Impulse Method for resolving collisions for linear velocities.

**2. Angular Collision Resolution(Involves Moment of Inertia for rotations)**
Unlike the above case, here, **we have to apply two impulses** to the bodies which are involved in a collision contact. One is the **Linear Impulse** which affect the bodies's linear velocities. The other is the **Angular Impulse** which affects the rotation of both the bodies.
-> Without rotation, all the points on the rigidbody move with the same linear velocity since there is no angular velocity.
-> With rotation, it matters where the point is located on the body. Because every point of the rigidbody will have different velocities depending on where they are located with respect to the body's center of mass. **The velocity of such a point is: (W X R) R is the vector which originates from its center of mass and points towards the point that we are calulating the velocity for and W is the angular velocity of the body.** Basically, the velocity of a point p will be different from the velocity of its center of mass. How different it is depends on where it is located on the body with respect to its center of mass(R Vector). Since this is a Cross Product, the velocity vector of the point is perpendicular to both the RVector and the (WVector).

Whenever we want to resolve collisions, we have to find the velocities of the bodies AFTER the collision doing some operations on the velocities of the bodies BEFORE the collision. In this case, we have to calculate the BEFORE and AFTER velocities of the specific POINTS OF CONTACT of the two bodies.
We compute the POINT VELOCITY of each body by adding the linear velocity due to the motion of their center of mass plus the velocity of the POINT caused by the rotation of the body.
**Va = va + (Wa X Ra)** - Here Va is the velocity vector of the point in body A, va is its center of mass's linear velocity Wa is the angular velocity and Ra is the distance vector from the center of body a to the point of contact in the collision. The velocity of the point in Body B is : **Vb = vb + (Wb X Rb)** These velocities are of the points in body A and B before the collision and these are the velocites of the points which are involved in the collision.
We have to find linear velocity and angular velocity of the bodies AFTER the collision
**v'a = va - J.N/Ma and v'b = vb + J.N/Mb** <--(1)
**w'a = wa - (Cross(Ra, J).N / Ia) and w'b = wb + (Cross(Rb, J).N / Ib)** Here Ia and Ib are the moment of inertias of the bodies. and N is the collision Normal. Ra and Rb are the distance vectors from the respective com of the bodies to the points of contact. <-- (2)

Therefore, the point velocity of the body A involved in the collision is:
**V'a = v'a + Cross(w'a, Ra)** <-- (3)

Putting (1) and (2) in (3)
V'a = (va - J.N / Ma) + (w'a X Ra)
V'a = (va - J.N / Ma) + ( (wa - ((Ra X J).N) / Ia) X Ra )
V'a = (va - J.N / Ma) + (wa X Ra) - ((Ra X J).N) / Ia) X Ra.
V'a = (va + (wa X Ra)) - (J.N / Ma) - ((Ra X J).N) / Ia) X Ra.
(va + (wa X Ra)) is the old velocity of the point BEFORE the collision, also called Va
Therefore,
V'a = Va - (J.N / Ma) - ((Ra X J).N) / Ia) X Ra.
**V'a = Va - (J.N / Ma) - J*(((Ra X N) / Ia) X Ra).** <-- (4)
Similarly for point on body B:
**V'b = Vb + (J.N / Mb) + J*(((Rb X N) / Ib) X Rb).** <-- (5)


for elastic collisions, we have this formula
V'rel.N = -E * Vrel.N
**(V'a - V'b).N = -E * (Va - Vb).N** <-- (6)
which is to say that the relative velocity (Va - Vb) along the collision normal after the collision is negative of the product of the coefficient of restitution(E) and the old relative velocity of the bodies along the collision normal BEFORE the collision

Putting (4) and (5) in (6), we get after simplifying:
(Va - Vb).N - J*(N.N/Ma + N.N/Mb) - J*( ((Ra X N) X Ra)/Ia + ((Rb X N) X Rb)/Ib ).N = -E * (Va - Vb).N
(1 + E)*(Va - Vb).N = J*(1/Ma + 1/Mb + ((Ra X N) X Ra.N)/Ia + ((Rb X N) X Rb.N)/Ib) --> Here, N.N = 1 since N is unit vector.
Let's look at this: ((Ra X N) X Ra).N, In vectors, there is a scalar triple product which is **(A X B).C which equals (B X C).A**
In this case, (Ra X N) is A, Ra is B and N is C. **(B X C).A is (Ra X N).(Ra X N) which is (Ra X N)^2**
Putting this in our calculation, we get:
(1 + E)*(Va - Vb).N = J*(1/Ma + 1/Mb + ((Ra X N).(Ra X N))/Ia + ((Rb X N).(Rb X N))/Ib)
so **J = (1 + E)*(Va - Vb).N / (1/Ma + 1/Mb + (Ra X N)^2/Ia + (Rb X N)^2/Ib)**

This is the Impulse we apply to both the bodies in opposite directions. This is the magnitude of the impulse. The direction of the impulse is along the collision normal.

When applying this impulse to both the bodies, we calculate the final linear velocity AND the angular velocity of the body.
for the linear velocity: V += J / M         --> M is the mass.
for the angular velocity: W += (R X J) / I  --> I is the moment of inertia. R is the distance vector from the body center
                                                to the point of contact in the collision





This is the velocity of the point P which came in contact with another body AFTER the collision. This is the formula we use to resolve the collision using impulses to move the bodies away from each other after the collision. The only difference here is that we apply linear as well as angular impulses to the said bodies.







