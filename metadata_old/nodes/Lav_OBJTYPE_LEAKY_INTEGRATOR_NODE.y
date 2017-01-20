properties:
  Lav_LEAKY_INTEGRATOR_LEAKYNESS:
    name: leakyness
    type: double
    default: 1.0
    range: [0.0, 1.0]
    doc_description: |
      The leakyness (or time constant) of the integrator.
      
      If you feed the leaky integrator a constant signal of 0, then this property's value is the percent decrease as observed after 1 second.
inputs:
  - [constructor, "A signal to integrate."]
outputs:
  - [constructor, "The integral of the signal."]
doc_name: leaky integrator
doc_description: |
  A leaky integrator.
  Leaky integrators integrate their input signals, while leaking over time.
  Introducing the leak allows for avoiding DC offset problems.
  If you feed this node a signal that is zero, it will slowly decrease the output in accordance with the {{"Lav_LEAKY_INTEGRATOR_LEAKYNESS"|property}} property.