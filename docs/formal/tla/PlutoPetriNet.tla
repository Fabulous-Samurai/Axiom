--------------------------- MODULE PlutoPetriNet ---------------------------
EXTENDS Naturals, Sequences, FiniteSets

CONSTANT NumExperts

VARIABLE places, tokens

(* Petri Net Places *)
(* P1: Task Queue (Incoming tasks) *)
(* P2: Processing (Active expert work) *)
(* P3: Completed (Tasks finished) *)
(* P4: AvailableExperts (Ready to work) *)

vars == <<places, tokens>>

TypeOK ==
    /\ places \subseteq {"P1", "P2", "P3", "P4"}
    /\ tokens \in [places -> Nat]

Init ==
    /\ places = {"P1", "P2", "P3", "P4"}
    /\ tokens = [P1 |-> 10, P2 |-> 0, P3 |-> 0, P4 |-> NumExperts]

(* Transitions *)
Dispatch ==
    /\ tokens["P1"] > 0
    /\ tokens["P4"] > 0
    /\ tokens' = [tokens EXCEPT !["P1"] = @ - 1, 
                               !["P4"] = @ - 1,
                               !["P2"] = @ + 1]

Complete ==
    /\ tokens["P2"] > 0
    /\ tokens' = [tokens EXCEPT !["P2"] = @ - 1, 
                               !["P4"] = @ + 1,
                               !["P3"] = @ + 1]

Next == Dispatch \/ Complete

Spec == Init /\ [][Next]_vars

(* Safety Invariant: Number of experts is conserved *)
ExpertConservation ==
    tokens["P2"] + tokens["P4"] = NumExperts

(* Liveness: Eventually all tasks in P1 move to P3 *)
TasksEventuallyDone ==
    WF_vars(Next) => <>(tokens["P1"] = 0 /\ tokens["P2"] = 0)

=============================================================================
