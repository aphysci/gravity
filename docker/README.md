# Docker Gravity Builds

## Local Builds
To use these files to build and test for each ubuntu version locally see the comments in the top of each Dockerfile

## To update the Github CI Docker images use the following instructions:
- docker login to ghcr.io using a personal access token following the instructions here: <br>
    [https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-container-registry](https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-container-registry)
- In the gravity root directory run: (replace focal with intended version name) <br>
  ```[bash]
  $ docker build --target base -t ghcr.io/aphysci/gravity_base:focal --rm -f docker/Dockerfile.focal .
  $ docker push ghcr.io/aphysci/gravity_base:focal
  ```