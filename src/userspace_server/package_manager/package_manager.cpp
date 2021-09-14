//
// Created by lukemartinlogan on 9/12/21.
//

#include <labstor/types/userspace_client/client.h>
#include <labstor/types/basics.h>
#include <labstor/types/package.h>

int main() {
    labstor::LabStorClientContext client;
    labstor::queue_pair *qp;

    client.Connect();
    client.LoadPackage("PackageManager");
    qp = client.GetQueuePair("PackageManager");
    package_request = (package_request*)qp.AllocRequest();
}