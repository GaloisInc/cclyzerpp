Docker
======

cclyzer++ ships with a ``Dockerfile`` with three targets:

- `dev` has all the tools needed to develop cclyzer++
- `build` is an intermediate image that builds cclyzer++
- `dist` contains the built cclyzer++ artifacts and their runtime dependencies

You can pull pre-built images from GHCR like so:

.. code-block:: bash

  docker pull ghcr.io/galoisinc/cclyzerpp-dev:main

