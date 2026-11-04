# InsureRisk-Linux
> High-Performance Actuarial Risk Evaluation & Monte Carlo Simulation Engine in C++17

**InsureRisk-Linux** is a modular, high-performance C++17 actuarial software system designed for Linux/WSL environments. It automates policyholder risk evaluation, stochastic loss forecasting via multi-threaded Monte Carlo simulations, and risk-adjusted premium pricing. The project emphasizes robust software architecture, stringent input validation, concurrency safety, and comprehensive testing methodologies.

---

## 🚀 Key Features
* **Encapsulated Policy Management:** Object-oriented representation of policyholders with rigorous boundary validation (`isValid()`).
* **Advanced Risk Scoring:** Composite risk evaluation factoring in credit history, past claims frequency, and geographic risk multipliers.
* **Concurrent Monte Carlo Stress Testing:** Stochastic loss modeling utilizing log-normal distributions and parallel execution via `std::async` and `std::future`.
* **Actuarial Pricing Engine:** Calculates baseline premiums (loss + expense loading + profit margin) and risk-adjusted premiums backed by Value-at-Risk ($\text{VaR}_{95\%}$) capital buffers.
* **Rigorous QA Pipeline:** Comprehensive white-box unit tests and black-box Boundary Value Analysis (BVA) powered by Google Test (GTest).

---

## 🏛️ System Architecture

The application is structured into four core decoupled modules:

| Component | Responsibility |
| :--- | :--- |
| **`Policyholder`** | Manages policyholder attributes (age, credit score, geographic zone, asset value, past claims) and enforces data integrity. |
| **`RiskEvaluator`** | Pure static logic class computing composite risk scores and expected annual losses. |
| **`MonteCarloSimulator`** | Multi-threaded simulation engine using Mersenne Twister (`std::mt19937`) to model loss distributions and compute Value-at-Risk. |
| **`ActuarialEngine`** | Computes baseline pricing models and capital-adjusted risk premiums. |

---

## 🛠️ Tech Stack
* **Language:** C++17
* **Build System:** CMake (with automated Google Test fetching via `FetchContent`)
* **Concurrency:** C++ Standard Library Concurrency (`std::async`, `std::vector`)
* **Testing Framework:** Google Test (GTest)

---

## 🧪 Verification & Testing Methodology
The application undergoes rigorous multi-tier testing:
1. **White-Box Unit Testing:** Verifies internal calculation logic, risk weight multipliers, and mathematical correctness using GTest.
2. **Black-Box Boundary Value Analysis (BVA):** Tests edge-case inputs (e.g., credit score boundaries at 299/300 and 850/851, negative asset values, underage limits) to guarantee bulletproof input validation.
3. **Integration Pipeline Testing:** Ensures seamless data flow from raw policy parameters through risk scoring, multi-threaded simulation, and final pricing.

---

## 📥 Getting Started & Build Instructions

### Prerequisites
Ensure you have a C++17 compatible compiler (GCC 9+ or Clang), CMake (version 3.14+), and Make installed on your Linux or WSL environment.

### 1. Clone & Setup Build Directory
```bash
cd ~/projects
git clone <repository-url> InsureRisk-Linux
cd InsureRisk-Linux
mkdir -p build && cd build