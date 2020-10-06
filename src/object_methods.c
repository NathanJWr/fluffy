/* Set up the function tables for each object type */

void InitObjectMethodEnvs() {
  InitObjectStringEnv();
  InitObjectArrayEnv();
  InitObjectSystemEnv();
}
