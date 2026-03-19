---
id: greedy-fitness-generation-retry
type: pattern
pillar: developer
---

# Pattern: Greedy Fitness Generation Retry

## 1. Description
A pattern for ensuring procedurally generated objects meet a minimum quality "fitness" floor without resorting to artificial score inflation. It combines a proactive "greedy" selection strategy (choosing the best components first) with a reactive retry loop that discards substandard candidates.

## 2. Structure
1. **Constraint Set**: Define a hard "fitness floor" (e.g., 0.5).
2. **Greedy Filling**: During generation, prioritize components that maximize the primary fitness metric (e.g., cargo modules for a freighter).
3. **Retry Loop**: Execute generation in a loop up to $N$ times.
4. **Validation**: Calculate the fitness of the candidate. If it meets the floor, return immediately.
5. **Fallthrough**: If the limit is reached, return the best candidate found so far (or error).

## 3. Platonic Implementation
```cpp
Candidate generateQualityObject(float minFitness, int maxAttempts) {
  Candidate bestCandidate;
  float bestScore = -1.0f;
  
  for (int i = 0; i < maxAttempts; ++i) {
    Candidate c = generateGreedy(); // Logic prioritized for fitness
    float score = calculateFitness(c);
    
    if (score >= minFitness) return c;
    if (score > bestScore) {
      bestScore = score;
      bestCandidate = c;
    }
  }
  return bestCandidate;
}
```
