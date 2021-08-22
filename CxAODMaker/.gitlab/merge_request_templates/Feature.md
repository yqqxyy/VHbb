# Contributing

To contribute to this project please start a merge request with the details of the planned work.

## Mere Request Description

Please describe the purpose of this merge request in some detail and what the specific feature being added will do. Reference and link to any relevant issues or merge requests (such as the issue in which this feature was first suggested).


# WIP

Unless this is a single commit merge request please create a [WIP merge request](https://docs.gitlab.com/ce/user/project/merge_requests/work_in_progress_merge_requests.html). Outline the work that will be done in this ongoing merge request and assign someone with Approver permissions to follow the merge request.

# Merge Requests Procedure

If you would like to make a merge request please:

0. Read the docs on the project workflow
1. Clone the project
2. Create a new feature branch off of the branch you wish to update
3. Commit your initial changes and push your branch to the `origin` [while setting the upstream](https://stackoverflow.com/q/5697750)
4. Start a [WIP merge request](https://docs.gitlab.com/ce/user/project/merge_requests/work_in_progress_merge_requests.html)
5. Continue work and test your changes locally
6. Continue to push your changes to the `origin` and once the new feature you're working on is complete check that all the CI tests pass and then request an Approver review your merge request

# Checklist Before Requesting Approver

- [ ] We have read the project workflow docs.
- [ ] We have checked the "Remove source branch when merge request is accepted" box. If not, we mention in the merge request why.
- [ ] We have checked the "Squash commits when merge request is accepted" box. If not, we mention in the merge request why.
- [ ] Our merge request passes all tests in the CI pipeline.
- [ ] We have removed "WIP" from the title of the merge request.
