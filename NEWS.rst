News for cam
============

This file lists the major changes between versions. For a more detailed list of
every change, see the Git log.

Latest
------
* tbd

6.0.0
-----
* Major: Updated to nalu version 4.

5.0.0
-----
* Minor: Added ``to_vector`` member function on ``capture_data``.
* Major: Change io_service from being a shared_ptr to being a raw pointer.

4.0.0
-----
* Major: Use new waf.
* Major: Remove sak dependencies,
* Major: Update remaining dependencies.

3.0.0
-----
* Major: Use shared_ptr io_service instead of raw pointer.

2.0.0
-----
* Minor: Introduced endian as a internal dependency.
* Major: Removed sak dependency
* Minor: TCP server example now runs until stopped. Clients can now connect and
  disconnect without causing the server to close.
* Major: Update nalu to version 2.
* Major: Rename c4m to cam.
* Major: Use nalu repository on gitlab (renamed from n4lu)

1.0.0
-----
* Major: Initial working release.
