# Bug Fix

This bug fix addresses the following issues and merge requests.

## Mere Request Description

Please describe the purpose of this merge request in some detail and what bug it fixes. Reference and link to any relevant issues or merge requests (such as the issue in which this bug was first discussed).

# WIP

Unless this is a single commit merge request please create a [WIP merge request](https://docs.gitlab.com/ce/user/project/merge_requests/work_in_progress_merge_requests.html). Outline the work that will be done in this ongoing merge request and assign someone with Approver permissions to follow the merge request.

# Merge Requests Procedure

If you would like to make a merge request please:

1. Read the docs on the project workflow
2. Clone the project
3. Create a new bug fix branch off of the branch you wish to update
4. Commit your initial changes and push your branch to the `origin` [while setting the upstream](https://stackoverflow.com/q/5697750)
5. Start a [WIP merge request](https://docs.gitlab.com/ce/user/project/merge_requests/work_in_progress_merge_requests.html)
6. Continue work and test your changes locally
7. Continue to push your changes to the `origin` and once the bug fix you're working on is complete check that all the CI tests pass and then request an Approver review your merge request

# Checklist Before Requesting Approver

- [ ] We have read the project workflow docs.
- [ ] We have documented in the merge request all issues and merge requests that mention this bug.
- [ ] We have checked the "Remove source branch when merge request is accepted" box. If not, we mention in the merge request why.
- [ ] We have checked the "Squash commits when merge request is accepted" box. If not, we mention in the merge request why.
- [ ] Our merge request passes all tests in the CI pipeline.
- [ ] We have removed "WIP" from the title of the merge request.
