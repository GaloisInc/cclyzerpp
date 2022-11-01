Docker
======

cclyzer++ ships with two dockerfiles in ``docker/``:

- ``dev.dockerfile`` has all the tools needed to develop cclyzer++
- ``dist.dockerfile`` contains cclyzer++ and its runtime dependencies

Obtaining Images
****************

.. _ghcr:

Pulling from GHCR
~~~~~~~~~~~~~~~~~

You can pull pre-built images from GHCR like so:

.. code-block:: bash

  docker pull ghcr.io/galoisinc/cclyzerpp-dev:main

The ``main`` tag refers to the most recent release. You can also specify a tag
or commit SHA of a previous release, e.g.,

.. code-block:: bash

  # The following are equivalent:
  docker pull ghcr.io/galoisinc/cclyzerpp-dev:v0.5.0
  docker pull ghcr.io/galoisinc/cclyzerpp-dev:47bea4c4223d653209d28b932824618128f47910

.. _docker-build:

Building Locally
~~~~~~~~~~~~~~~~

.. code-block:: bash

  cd docker
  DOCKER_BUILDKIT=1 docker build --tag=cclyzerpp-dev --file=dev.dockerfile ..

To build the ``dist`` image, first build cclyzer++ as described in :doc:`build`.
For example, to do so inside the ``dev`` image, run this from the source root:

.. code-block:: bash

  docker run \
    --rm \
    --mount type=bind,src=$PWD,target=/work \
    --workdir /work \
    "ghcr.io/galoisinc/cclyzerpp-dev:main" \
    bash -c "cmake -G Ninja -B build -S . && cmake --build build -j $(nproc)"

You should see files in ``build/``. Now, build the ``dist`` image:

.. code-block:: bash

  cd docker
  DOCKER_BUILDKIT=1 docker build --tag=cclyzerpp-dist --file=dist.dockerfile ..

Rationale
*********

.. note::
    This section is mainly of interest to developers of cclyzer++.

This section discusses the trade-offs of different choices made concerning the
Docker images.

Multiple Dockerfiles
~~~~~~~~~~~~~~~~~~~~

Using multiple Dockerfiles rather than a multi-stage build with different
``dev`` and ``dist`` stages enables the use of `multiple dockerignores
<https://github.com/moby/moby/issues/12886#issuecomment-480575928>`_. In a
multi-stage build, the dockerignore would have to allow any file used by any
stage, whereas with the current setup, each image has its own dockerignore.

Granular dockerignores cause faster builds, as evidenced by lines like the
following in the ``docker build`` output:

.. code-block::

  => transferring context: 2B                   0.0s

This shows that Docker didn't have to copy any files (the "build context") to
the Docker daemon, making the build that much faster. Furthermore, granular
dependencies cause more cache hits.
