callbacks: [processing]
doc_name: custom
doc_description: |
  This node's processing depends solely on a user-defined callback.
  It has a specific number of inputs and outputs which are aggregated into individual channels.
  The callback is then called for every block of audio.
  If implementing a custom node, it is required that you handle all communication yourself.
