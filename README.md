Setting up and running an experiment with GaiaOS and viewing the results:

Steps:

1. Clone the repository to your local machine.

2. Navigate to the "proto-Gaia-Main" folder.

3. Run builderman.bat [Windows] or builderman.sh [Linux/Mac]

4. Go to the "bin" folder and run "NT4.exe"

	Here are some example parameters to copy paste in for now, NT4 can be used as a CLI tool as well:
		testermon 4 25 100 100 0 9000 0.99 0.95 0.99 0.95 0.0 0.95 0.0 0.95 0 5 5 0.25 15 15 6

5. From the "bin" folder navigate to the "GaiaTesting" folder, then to the "viewer" folder.

6. Run render.bat [Windows] or render.sh [Linux] with the same experiment name from when you ran NT4.exe. In the example provided "testerman" is the experiment name, so here you type "render testerman" in the console.

If something doesn't work or there is a discrepency please let me know at briarfisk@gmail.com so the issue can be investigated and the reason figured out. 