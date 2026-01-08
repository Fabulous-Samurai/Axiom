"""
🎨 Complex Plot Test Cases for AXIOM GUI
Advanced visualization tests for the plotting system
"""

# ============================================================================
# 🌈 MULTI-FUNCTION PLOTS (Overlay Multiple Functions)
# ============================================================================

# 1. Sine vs Cosine comparison
plot: sin(x), cos(x), 0, 2*pi, -1.5, 1.5

# 2. Polynomial family comparison
plot: x, x^2, x^3, -2, 2, -8, 8

# 3. Exponential growth vs decay
plot: exp(x), exp(-x), -2, 2, 0, 8

# 4. Different amplitudes
plot: sin(x), 2*sin(x), 0.5*sin(x), 0, 2*pi, -2.5, 2.5

# 5. Frequency comparison
plot: sin(x), sin(2*x), sin(3*x), 0, 2*pi, -1.5, 1.5


# ============================================================================
# 🎢 COMPLEX MATHEMATICAL FUNCTIONS
# ============================================================================

# 6. Damped oscillation (physics/engineering)
plot: exp(-x/5)*sin(x), 0, 20, -1, 1

# 7. Beat frequency (audio/acoustics)
plot: sin(10*x)*sin(x), 0, 20, -1.5, 1.5

# 8. Gaussian (normal distribution curve)
plot: exp(-x^2/2), -4, 4, 0, 1.2

# 9. Mexican hat wavelet
plot: (1-x^2)*exp(-x^2/2), -4, 4, -0.5, 1

# 10. Sigmoid function (neural networks)
plot: 1/(1+exp(-x)), -6, 6, -0.1, 1.1

# 11. Hyperbolic tangent
plot: tanh(x), -4, 4, -1.5, 1.5

# 12. Cardinal sine (sinc function)
plot: sin(x)/x, -10, 10, -0.5, 1.2

# 13. Logarithmic spiral effect
plot: ln(abs(x)+1)*sin(x), 0, 10, -3, 3

# 14. Chirp signal (increasing frequency)
plot: sin(x^2), 0, 5, -1.5, 1.5

# 15. Lissajous-like curve (y-component)
plot: sin(2*x)*cos(3*x), 0, 2*pi, -1.5, 1.5


# ============================================================================
# 🔥 EXTREME FUNCTION BEHAVIORS
# ============================================================================

# 16. Very sharp peak
plot: 1/(1+100*x^2), -1, 1, 0, 1.2

# 17. Rapid oscillation
plot: sin(50*x), 0, pi, -1.5, 1.5

# 18. Multiple scales
plot: 0.1*sin(x) + 0.01*sin(100*x), 0, 2*pi, -0.15, 0.15

# 19. Discontinuity approximation
plot: atan(100*x), -1, 1, -2, 2

# 20. Devil's staircase approximation
plot: atan(sin(10*x)), -pi, pi, -2, 2


# ============================================================================
# 📊 STATISTICAL & PROBABILITY DISTRIBUTIONS
# ============================================================================

# 21. Cauchy distribution
plot: 1/(pi*(1+x^2)), -5, 5, 0, 0.4

# 22. Laplace distribution
plot: 0.5*exp(-abs(x)), -5, 5, 0, 0.6

# 23. Logistic distribution
plot: exp(x)/(1+exp(x))^2, -6, 6, 0, 0.3

# 24. Beta-like curve
plot: x^2*(1-x)^2, 0, 1, 0, 0.05


# ============================================================================
# 🎭 SPECIAL FUNCTION COMBINATIONS
# ============================================================================

# 25. Superposition of harmonics (Fourier-like)
plot: sin(x) + sin(3*x)/3 + sin(5*x)/5, 0, 4*pi, -2, 2

# 26. AM modulation
plot: (1+0.5*sin(x))*sin(10*x), 0, 4*pi, -2, 2

# 27. FM modulation
plot: sin(10*x + sin(x)), 0, 4*pi, -1.5, 1.5

# 28. Polynomial with trig
plot: x*sin(x), -10, 10, -10, 10

# 29. Exponential times trig
plot: exp(-x)*cos(10*x), 0, 5, -1, 1

# 30. Power law with oscillation
plot: x^0.5*sin(x), 0, 10, -4, 4


# ============================================================================
# 🌀 PARAMETRIC PLOTS (if supported)
# ============================================================================

# 31. Circle parametric: x(t)=cos(t), y(t)=sin(t), t: 0 to 2π
parametric: cos(t), sin(t), 0, 2*pi

# 32. Spiral: x(t)=t*cos(t), y(t)=t*sin(t)
parametric: t*cos(t), t*sin(t), 0, 4*pi

# 33. Lissajous: x(t)=sin(2t), y(t)=sin(3t)
parametric: sin(2*t), sin(3*t), 0, 2*pi

# 34. Ellipse: x(t)=2*cos(t), y(t)=sin(t)
parametric: 2*cos(t), sin(t), 0, 2*pi

# 35. Rose curve: x(t)=cos(3t)*cos(t), y(t)=cos(3t)*sin(t)
parametric: cos(3*t)*cos(t), cos(3*t)*sin(t), 0, 2*pi


# ============================================================================
# 🔬 SCIENTIFIC & ENGINEERING FUNCTIONS
# ============================================================================

# 36. Blackbody radiation (Planck-like)
plot: x^3/(exp(x)-1), 0.1, 10, 0, 1

# 37. Fermi-Dirac distribution
plot: 1/(1+exp((x-2)/0.5)), -2, 6, 0, 1.2

# 38. RC circuit response
plot: 1-exp(-x), 0, 5, 0, 1.2

# 39. RLC resonance curve
plot: 1/sqrt((x-5)^2 + 0.5), 0, 10, 0, 2

# 40. Lorentzian peak
plot: 1/(1+(x-5)^2), 0, 10, 0, 1.2


# ============================================================================
# 🎪 FRACTAL-LIKE & SELF-SIMILAR PATTERNS
# ============================================================================

# 41. Weierstrass-like function (pseudo-fractal)
plot: sin(x) + 0.5*sin(2*x) + 0.25*sin(4*x) + 0.125*sin(8*x), 0, 2*pi, -2, 2

# 42. Triangle wave approximation (Fourier series)
plot: sin(x) - sin(3*x)/9 + sin(5*x)/25, 0, 4*pi, -1.5, 1.5

# 43. Sawtooth approximation
plot: sin(x) + sin(2*x)/2 + sin(3*x)/3, 0, 4*pi, -2, 2


# ============================================================================
# 💫 QUANTUM MECHANICS INSPIRED
# ============================================================================

# 44. Particle in box wavefunction
plot: sin(pi*x), 0, 1, -1.5, 1.5

# 45. Harmonic oscillator ground state
plot: exp(-x^2), -3, 3, 0, 1.2

# 46. First excited state
plot: x*exp(-x^2/2), -3, 3, -1, 1


# ============================================================================
# 🎯 STRESS TESTS FOR RENDERING
# ============================================================================

# 47. Very high frequency (renderer stress test)
plot: sin(100*x), 0, pi, -1.5, 1.5

# 48. Extremely steep gradient
plot: atan(1000*x), -0.1, 0.1, -2, 2

# 49. Narrow spike
plot: exp(-100*x^2), -0.5, 0.5, 0, 1.2

# 50. Multiple scales combined
plot: sin(x) + 0.1*sin(10*x) + 0.01*sin(100*x), 0, 2*pi, -1.2, 1.2


# ============================================================================
# 🌟 BEAUTIFUL MATHEMATICAL CURVES
# ============================================================================

# 51. Witch of Agnesi
plot: 1/(1+x^2), -5, 5, 0, 1.2

# 52. Butterfly curve (projection)
plot: sin(x)*(exp(cos(x)) - 2*cos(4*x) - sin(x/12)^5), 0, 12*pi, -4, 4

# 53. Heart curve approximation
plot: sqrt(1-abs(x)) + sqrt(abs(x)), -1, 1, 0, 2

# 54. Cardioid-like
plot: (1-cos(x))/2, 0, 2*pi, 0, 1

# 55. Nephroid-like
plot: (1-cos(x))^2/4, 0, 2*pi, 0, 1


# ============================================================================
# 📈 COMPARATIVE GROWTH RATES
# ============================================================================

# 56. Polynomial vs exponential growth
plot: x^3, exp(x), 0, 5, 0, 100

# 57. Log vs power growth
plot: ln(x), x^0.5, 1, 10, 0, 4

# 58. Different power laws
plot: x^0.5, x, x^2, x^3, 0, 2, 0, 8


# ============================================================================
# 🎨 ARTISTIC / AESTHETIC PATTERNS
# ============================================================================

# 59. Interference pattern
plot: sin(x)*sin(x/7), 0, 50, -1.5, 1.5

# 60. Moiré-like pattern
plot: sin(x) + sin(1.1*x), 0, 100, -2.5, 2.5

# 61. Beating waves
plot: cos(10*x)*cos(11*x), 0, 20, -1.5, 1.5


# ============================================================================
# 🔮 TIPS FOR USING THESE TESTS
# ============================================================================

"""
USAGE INSTRUCTIONS:
===================

1. Copy individual lines (without the #) into the AXIOM GUI input
2. Use mode switching if needed: :mode plot
3. For parametric plots, ensure parametric mode is supported
4. Adjust x-ranges and y-ranges based on your screen resolution
5. Use these for:
   - Testing rendering speed (persistent subprocess should be <10ms)
   - Verifying mathematical accuracy
   - Stress-testing the plot engine
   - Creating beautiful visualizations
   - Teaching calculus/physics concepts

EXPECTED BEHAVIOR:
==================
- Simple plots: <10ms rendering with persistent subprocess
- Complex plots: 10-50ms depending on function complexity
- Multi-function overlays: Slightly slower but still <100ms
- Parametric plots: Similar to regular plots

TROUBLESHOOTING:
================
- If plot command doesn't work, try: :mode plot
- Use mod(a, b) instead of a%b for modulus
- For division by zero, expect graceful handling
- Asymptotes should be visible or handled smoothly
"""
